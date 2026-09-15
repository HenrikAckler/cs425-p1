#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "harness/unity.h"
#include "../src/lab.h"
#include "../src/protocolHelpers.h"
#include "../src/session.h"


void setUp(void) {
  printf("Setting up tests...\n");
}

void tearDown(void) {
  printf("Tearing down tests...\n");
}

void test_get_greeting(void) {
  // Cover normal formatting, NULL rejection, and the empty-name boundary case.
  char *greeting = get_greeting("Alice");
  TEST_ASSERT_NOT_NULL(greeting);
  TEST_ASSERT_EQUAL_STRING("Hello, Alice!", greeting);
  free(greeting); // Free the allocated memory for the greeting

  greeting = get_greeting(NULL);
  TEST_ASSERT_NULL(greeting);

  greeting = get_greeting("");
  TEST_ASSERT_NOT_NULL(greeting);
  TEST_ASSERT_EQUAL_STRING("Hello, !", greeting);
  free(greeting);
}

void test_protocol_helpers(void) {
  // Cover line-injection checks, reply parsing, command framing, and DATA encoding.
  TEST_ASSERT_TRUE(contains_crlf("bad\nvalue"));
  TEST_ASSERT_TRUE(contains_crlf("bad\rvalue"));
  TEST_ASSERT_FALSE(contains_crlf("safe value"));
  TEST_ASSERT_FALSE(contains_crlf(NULL));

  TEST_ASSERT_EQUAL_INT(250, parse_reply_code("250 OK\r\n"));
  TEST_ASSERT_EQUAL_INT(250, parse_reply_code("250-continue\r\n"));
  TEST_ASSERT_EQUAL_INT(-1, parse_reply_code(NULL));
  TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("25 OK\r\n"));
  TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("250?bad\r\n"));
  TEST_ASSERT_TRUE(reply_is_final("250 OK\r\n"));
  TEST_ASSERT_FALSE(reply_is_final("250-continue\r\n"));
  TEST_ASSERT_FALSE(reply_is_final("bad\r\n"));

  char *command = build_command("HELO localhost");
  TEST_ASSERT_NOT_NULL(command);
  TEST_ASSERT_EQUAL_STRING("HELO localhost\r\n", command);
  free(command);
  TEST_ASSERT_NULL(build_command(NULL));
  TEST_ASSERT_NULL(build_command("HELO\r\n"));

  char *body = dot_stuff_body("first\n.second\r\nthird");
  TEST_ASSERT_NOT_NULL(body);
  TEST_ASSERT_EQUAL_STRING("first\r\n..second\r\nthird\r\n", body);
  free(body);
  TEST_ASSERT_NULL(dot_stuff_body(NULL));
  TEST_ASSERT_NULL(dot_stuff_body("bad\rbody"));

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
  TEST_ASSERT_NULL(build_data_payload("bad\nfrom", "to", "subject", "body"));
  TEST_ASSERT_NULL(build_data_payload("from", "to", "bad\rsubject", "body"));
}

typedef struct {
  const char *replies;
  size_t reply_offset;
  size_t read_chunk;
  char writes[2048];
  size_t write_length;
  size_t write_chunk;
} scripted_transport;

ssize_t scripted_read(void *context, void *buffer, size_t length) {
  scripted_transport *script = context;
  size_t remaining = strlen(script->replies) - script->reply_offset;
  size_t count = remaining < script->read_chunk ? remaining : script->read_chunk;
  if (count > length) {
    count = length;
  }
  if (count == 0U) {
    return 0;
  }
  memcpy(buffer, script->replies + script->reply_offset, count);
  script->reply_offset += count;
  return (ssize_t) count;
}

ssize_t scripted_write(void *context, const void *buffer, size_t length) {
  scripted_transport *script = context;
  size_t count = length < script->write_chunk ? length : script->write_chunk;
  if (script->write_length + count >= sizeof(script->writes)) {
    return -1;
  }
  memcpy(script->writes + script->write_length, buffer, count);
  script->write_length += count;
  script->writes[script->write_length] = '\0';
  return (ssize_t) count;
}

void test_session_with_split_transport(void) {
  // Feed fragmented replies and short writes to verify buffering and write_all.
  scripted_transport script = {
      "220 ready\r\n"
      "250 helo\r\n"
      "250 mail\r\n"
      "250 rcpt\r\n"
      "354 send\r\n"
      "250 queued\r\n"
      "221 bye\r\n",
      0U, 3U, "", 0U, 5U};
  session_transport transport = {scripted_read, scripted_write, &script};

  TEST_ASSERT_EQUAL_INT(0, session_run(&transport, "from@example.com",
                                       "to@example.com", "subject", "body",
                                       "localhost"));
  TEST_ASSERT_EQUAL_STRING(
      "HELO localhost\r\n"
      "MAIL FROM:<from@example.com>\r\n"
      "RCPT TO:<to@example.com>\r\n"
      "DATA\r\n"
      "From: from@example.com\r\n"
      "To: to@example.com\r\n"
      "Subject: subject\r\n"
      "\r\n"
      "body\r\n"
      ".\r\n"
      "QUIT\r\n", script.writes);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_get_greeting);
  RUN_TEST(test_protocol_helpers);
  RUN_TEST(test_session_with_split_transport);
  return UNITY_END();
}
