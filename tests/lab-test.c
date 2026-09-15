#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness/unity.h"
#include "../src/protocolHelpers.h"
#include "../src/session.h"
#include "../src/socketTransport.h"

#define WRITE_BUFFER_SIZE 4096U

typedef struct {
    /* Replies model server-to-client bytes; writes capture client output. */
    const char *replies;
    size_t reply_offset;
    size_t read_chunk;
    size_t write_chunk;
    size_t read_calls;
    size_t write_calls;
    /* A nonzero call number makes the scripted transport fail from that call. */
    size_t fail_read_call;
    size_t fail_write_call;
    char writes[WRITE_BUFFER_SIZE];
    size_t write_length;
} scripted_transport;

static void script_init(scripted_transport *script, const char *replies,
                        size_t read_chunk, size_t write_chunk)
{
    memset(script, 0, sizeof(*script));
    script->replies = replies;
    script->read_chunk = read_chunk;
    script->write_chunk = write_chunk;
}

static ssize_t scripted_read(void *context, void *buffer, size_t length)
{
    scripted_transport *script = context;
    ++script->read_calls;
    if (script->fail_read_call != 0U &&
        script->read_calls >= script->fail_read_call) {
        return -1;
    }

    size_t reply_length = strlen(script->replies);
    if (script->reply_offset >= reply_length) {
        return 0;
    }

    size_t count = reply_length - script->reply_offset;
    if (count > script->read_chunk) {
        count = script->read_chunk;
    }
    if (count > length) {
        count = length;
    }
    memcpy(buffer, script->replies + script->reply_offset, count);
    script->reply_offset += count;
    return (ssize_t) count;
}

static ssize_t scripted_write(void *context, const void *buffer, size_t length)
{
    scripted_transport *script = context;
    ++script->write_calls;
    if (script->fail_write_call != 0U &&
        script->write_calls >= script->fail_write_call) {
        return -1;
    }

    size_t count = length;
    if (count > script->write_chunk) {
        count = script->write_chunk;
    }
    if (count > sizeof(script->writes) - script->write_length - 1U) {
        return -1;
    }
    memcpy(script->writes + script->write_length, buffer, count);
    script->write_length += count;
    script->writes[script->write_length] = '\0';
    return (ssize_t) count;
}

static session_transport script_as_session(scripted_transport *script)
{
    session_transport transport = {
        scripted_read,
        scripted_write,
        script
    };
    return transport;
}

static int run_session(scripted_transport *script)
{
    session_transport transport = script_as_session(script);
    return session_run(&transport, "from@example.com", "to@example.com",
                       "subject", "body", "localhost");
}

static const char *expected_writes(void)
{
    /* The complete wire transcript for a successful SMTP session. */
    return "HELO localhost\r\n"
           "MAIL FROM:<from@example.com>\r\n"
           "RCPT TO:<to@example.com>\r\n"
           "DATA\r\n"
           "From: from@example.com\r\n"
           "To: to@example.com\r\n"
           "Subject: subject\r\n"
           "\r\n"
           "body\r\n"
           ".\r\n"
           "QUIT\r\n";
}

void setUp(void) {}
void tearDown(void) {}

void test_contains_crlf(void)
{
    /* CR and LF are rejected because they can inject protocol lines. */
    TEST_ASSERT_FALSE(contains_crlf(NULL));
    TEST_ASSERT_FALSE(contains_crlf(""));
    TEST_ASSERT_FALSE(contains_crlf("safe value"));
    TEST_ASSERT_TRUE(contains_crlf("bad\rvalue"));
    TEST_ASSERT_TRUE(contains_crlf("bad\nvalue"));
    TEST_ASSERT_TRUE(contains_crlf("bad\r\nvalue"));
}

void test_reply_parsing_and_framing(void)
{
    /* SMTP replies require three digits followed by a space or continuation dash. */
    TEST_ASSERT_EQUAL_INT(200, parse_reply_code("200 OK\r\n"));
    TEST_ASSERT_EQUAL_INT(599, parse_reply_code("599-last\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code(NULL));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code(""));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("25 OK\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("250?bad\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("2a0 OK\r\n"));

    TEST_ASSERT_TRUE(reply_is_final("250 OK\r\n"));
    TEST_ASSERT_FALSE(reply_is_final("250-more\r\n"));
    TEST_ASSERT_FALSE(reply_is_final("250\r\n"));
    TEST_ASSERT_FALSE(reply_is_final(NULL));

    char *command = build_command("HELO localhost");
    TEST_ASSERT_NOT_NULL(command);
    TEST_ASSERT_EQUAL_STRING("HELO localhost\r\n", command);
    free(command);
    command = build_command("");
    TEST_ASSERT_NOT_NULL(command);
    TEST_ASSERT_EQUAL_STRING("\r\n", command);
    free(command);
    TEST_ASSERT_NULL(build_command(NULL));
    TEST_ASSERT_NULL(build_command("HELO\r\n"));
    TEST_ASSERT_NULL(build_command("HELO\nQUIT"));
}

void test_dot_stuffing(void)
{
    /* Body lines beginning with a period must be prefixed with another period. */
    char *body = dot_stuff_body("");
    TEST_ASSERT_NOT_NULL(body);
    TEST_ASSERT_EQUAL_STRING("", body);
    free(body);

    body = dot_stuff_body("first\n.second\r\n..third\n.");
    TEST_ASSERT_NOT_NULL(body);
    TEST_ASSERT_EQUAL_STRING("first\r\n..second\r\n...third\r\n..\r\n", body);
    free(body);

    body = dot_stuff_body("already terminated\r\n");
    TEST_ASSERT_NOT_NULL(body);
    TEST_ASSERT_EQUAL_STRING("already terminated\r\n", body);
    free(body);

    TEST_ASSERT_NULL(dot_stuff_body(NULL));
    TEST_ASSERT_NULL(dot_stuff_body("bare\rreturn"));
}

void test_data_payload(void)
{
    /* DATA combines fixed headers, normalized body lines, and the terminator. */
    char *payload = build_data_payload("from@example.com", "to@example.com",
                                       "subject", "first\n.second");
    TEST_ASSERT_NOT_NULL(payload);
    TEST_ASSERT_EQUAL_STRING(
        "From: from@example.com\r\n"
        "To: to@example.com\r\n"
        "Subject: subject\r\n"
        "\r\n"
        "first\r\n"
        "..second\r\n"
        ".\r\n", payload);
    free(payload);

    payload = build_data_payload("from", "to", "", "");
    TEST_ASSERT_NOT_NULL(payload);
    TEST_ASSERT_EQUAL_STRING("From: from\r\nTo: to\r\nSubject: \r\n\r\n.\r\n",
                             payload);
    free(payload);

    TEST_ASSERT_NULL(build_data_payload(NULL, "to", "subject", "body"));
    TEST_ASSERT_NULL(build_data_payload("bad\nfrom", "to", "subject", "body"));
    TEST_ASSERT_NULL(build_data_payload("from", "bad\rto", "subject", "body"));
    TEST_ASSERT_NULL(build_data_payload("from", "to", "bad\nsubject", "body"));
    TEST_ASSERT_NULL(build_data_payload("from", "to", "subject", "bad\rbody"));
}

void test_session_success_with_fragmentation_and_short_writes(void)
{
    /* One callback need not contain a complete line, and one write need not
       accept the complete command. */
    scripted_transport script;
    script_init(&script,
                "220 ready\r\n250 helo\r\n250 mail\r\n250 rcpt\r\n"
                "354 send\r\n250 queued\r\n221 bye\r\n", 3U, 5U);

    TEST_ASSERT_EQUAL_INT(0, run_session(&script));
    TEST_ASSERT_EQUAL_STRING(expected_writes(), script.writes);
    TEST_ASSERT_GREATER_THAN_UINT(1U, script.read_calls);
    TEST_ASSERT_GREATER_THAN_UINT(1U, script.write_calls);
}

void test_session_accepts_multiline_and_coalesced_replies(void)
{
    /* The reader must consume continuation lines while retaining later replies
       already present in the same transport buffer. */
    scripted_transport script;
    script_init(&script,
                "220-ready\r\n220 greeting\r\n"
                "250-helo\r\n250 capabilities\r\n"
                "250 mail\r\n250 rcpt\r\n354 data\r\n"
                "250 queued\r\n221 bye\r\n", 4096U, 4096U);

    TEST_ASSERT_EQUAL_INT(0, run_session(&script));
    TEST_ASSERT_EQUAL_STRING(expected_writes(), script.writes);
    TEST_ASSERT_EQUAL_UINT(strlen(script.replies), script.reply_offset);
}

void test_session_transmits_dot_stuffed_payload(void)
{
    /* Verify that the session uses the protocol helper's encoded DATA payload. */
    scripted_transport script;
    script_init(&script,
                "220 ready\r\n250 helo\r\n250 mail\r\n250 rcpt\r\n"
                "354 send\r\n250 queued\r\n221 bye\r\n", 7U, 4096U);
    session_transport transport = script_as_session(&script);

    TEST_ASSERT_EQUAL_INT(0, session_run(&transport, "from", "to", "subject",
                                         "one\n.starts with dot", "host"));
    TEST_ASSERT_NOT_NULL(strstr(script.writes, "one\r\n..starts with dot\r\n.\r\n"));
}

void test_session_rejects_oversized_unterminated_reply(void)
{
    /* A reply without CRLF cannot be accepted once the fixed receive buffer fills. */
    char replies[4097];
    memset(replies, 'x', sizeof(replies) - 1U);
    replies[sizeof(replies) - 1U] = '\0';
    scripted_transport script;
    script_init(&script, replies, 4096U, 4096U);

    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING("", script.writes);
}

static void assert_session_stops_at_failure(const char *replies,
                                            const char *expected_writes_prefix)
{
    /* Every SMTP error must stop the exchange before the next command. */
    scripted_transport script;
    script_init(&script, replies, 4096U, 4096U);
    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING(expected_writes_prefix, script.writes);
}

void test_session_rejects_each_unexpected_status(void)
{
    /* Exercise each required status checkpoint in the SMTP sequence. */
    assert_session_stops_at_failure("500 greeting\r\n", "");
    assert_session_stops_at_failure("220 ready\r\n550 helo\r\n",
                                    "HELO localhost\r\n");
    assert_session_stops_at_failure("220 ready\r\n250 helo\r\n550 mail\r\n",
                                    "HELO localhost\r\nMAIL FROM:<from@example.com>\r\n");
    assert_session_stops_at_failure("220 ready\r\n250 helo\r\n250 mail\r\n"
                                    "550 rcpt\r\n",
                                    "HELO localhost\r\nMAIL FROM:<from@example.com>\r\n"
                                    "RCPT TO:<to@example.com>\r\n");
    assert_session_stops_at_failure("220 ready\r\n250 helo\r\n250 mail\r\n"
                                    "250 rcpt\r\n250 data\r\n",
                                    "HELO localhost\r\nMAIL FROM:<from@example.com>\r\n"
                                    "RCPT TO:<to@example.com>\r\nDATA\r\n");
    assert_session_stops_at_failure("220 ready\r\n250 helo\r\n250 mail\r\n"
                                    "250 rcpt\r\n354 data\r\n550 queued\r\n",
                                    "HELO localhost\r\nMAIL FROM:<from@example.com>\r\n"
                                    "RCPT TO:<to@example.com>\r\nDATA\r\n"
                                    "From: from@example.com\r\nTo: to@example.com\r\n"
                                    "Subject: subject\r\n\r\nbody\r\n.\r\n");
    assert_session_stops_at_failure("220 ready\r\n250 helo\r\n250 mail\r\n"
                                    "250 rcpt\r\n354 data\r\n250 queued\r\n"
                                    "550 quit\r\n",
                                    "HELO localhost\r\nMAIL FROM:<from@example.com>\r\n"
                                    "RCPT TO:<to@example.com>\r\nDATA\r\n"
                                    "From: from@example.com\r\nTo: to@example.com\r\n"
                                    "Subject: subject\r\n\r\nbody\r\n.\r\n"
                                    "QUIT\r\n");
}

void test_session_rejects_malformed_and_mismatched_replies(void)
{
    /* Continuation replies must be syntactically valid and keep one status code. */
    assert_session_stops_at_failure("bad reply\r\n", "");
    assert_session_stops_at_failure("220-ready\r\n250 wrong-code\r\n", "");
    assert_session_stops_at_failure("220-ready\r\n221 final\r\n", "");
    assert_session_stops_at_failure("220 ready", "");
}

void test_session_rejects_transport_failures_and_invalid_inputs(void)
{
    /* Simulate both a peer failure while reading and a failed DATA write. */
    scripted_transport script;
    script_init(&script, "220 ready\r\n250 helo\r\n", 11U, 4096U);
    script.fail_read_call = 2U;
    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING("HELO localhost\r\n", script.writes);

    script_init(&script, "220 ready\r\n250 helo\r\n250 mail\r\n250 rcpt\r\n"
                         "354 data\r\n", 4096U, 4096U);
    script.fail_write_call = 5U;
    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING("HELO localhost\r\n"
                             "MAIL FROM:<from@example.com>\r\n"
                             "RCPT TO:<to@example.com>\r\n"
                             "DATA\r\n", script.writes);

    session_transport transport = script_as_session(&script);
    TEST_ASSERT_EQUAL_INT(-1, session_run(NULL, "from", "to", "subject",
                                          "body", "host"));
    transport.read = NULL;
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "body", "host"));
    transport = script_as_session(&script);
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, NULL, "to", "subject",
                                          "body", "host"));
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "body", NULL));
}

void test_socket_transport_contract(void)
{
    /* The socket adapter is tested without opening a real SMTP connection. */
    socket_transport socket = {.descriptor = 123};
    TEST_ASSERT_EQUAL_INT(-1, socket_transport_connect(NULL, "localhost", "25"));
    TEST_ASSERT_EQUAL_INT(-1, socket_transport_connect(&socket, NULL, "25"));
    TEST_ASSERT_EQUAL_INT(-1, socket_transport_connect(&socket, "localhost", NULL));

    TEST_ASSERT_EQUAL_INT(-1, socket_transport_connect(&socket,
                                                       "invalid.invalid", "25"));
    TEST_ASSERT_EQUAL_INT(-1, socket.descriptor);

    socket_transport_close(&socket);
    socket_transport_close(NULL);
    session_transport transport = socket_transport_as_session(&socket);
    TEST_ASSERT_NOT_NULL(transport.read);
    TEST_ASSERT_NOT_NULL(transport.write);
    TEST_ASSERT_EQUAL(&socket, transport.context);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_contains_crlf);
    RUN_TEST(test_reply_parsing_and_framing);
    RUN_TEST(test_dot_stuffing);
    RUN_TEST(test_data_payload);
    RUN_TEST(test_session_success_with_fragmentation_and_short_writes);
    RUN_TEST(test_session_accepts_multiline_and_coalesced_replies);
    RUN_TEST(test_session_transmits_dot_stuffed_payload);
    RUN_TEST(test_session_rejects_oversized_unterminated_reply);
    RUN_TEST(test_session_rejects_each_unexpected_status);
    RUN_TEST(test_session_rejects_malformed_and_mismatched_replies);
    RUN_TEST(test_session_rejects_transport_failures_and_invalid_inputs);
    RUN_TEST(test_socket_transport_contract);
    return UNITY_END();
}
