#include "session.h"

#include "protocolHelpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SESSION_BUFFER_SIZE 4096U
#define SESSION_MAX_REPLY_LINES 100U

typedef struct {
    const session_transport *transport;
    char buffer[SESSION_BUFFER_SIZE];
    size_t start;
    size_t end;
} receiver;

/* Refill the fixed receive buffer only when it has no complete CRLF line. */
static char *read_line(receiver *input)
{
    for (;;) {
        for (size_t index = input->start; index + 1U < input->end; ++index) {
            if (input->buffer[index] == '\r' && input->buffer[index + 1U] == '\n') {
                size_t line_length = index - input->start + 2U;
                char *line = malloc(line_length + 1U);
                if (line == NULL) {
                    return NULL;
                }
                memcpy(line, input->buffer + input->start, line_length);
                line[line_length] = '\0';
                input->start = index + 2U;
                return line;
            }
        }

        if (input->start > 0U) {
            size_t remaining = input->end - input->start;
            memmove(input->buffer, input->buffer + input->start, remaining);
            input->start = 0U;
            input->end = remaining;
        }

        if (input->end == SESSION_BUFFER_SIZE) {
            return NULL;
        }

        ssize_t count = input->transport->read(input->transport->context,
                                                input->buffer + input->end,
                                                SESSION_BUFFER_SIZE - input->end);
        if (count <= 0) {
            return NULL;
        }
        input->end += (size_t) count;
    }
}

/* Consume continuation lines and retain the final line for diagnostics. */
static int read_reply(receiver *input, int *code, char **final_line)
{
    char *line = read_line(input);
    if (line == NULL) {
        return -1;
    }

    int first_code = parse_reply_code(line);
    if (first_code < 0) {
        free(line);
        return -1;
    }

    size_t line_count = 1U;
    while (!reply_is_final(line)) {
        free(line);
        if (line_count >= SESSION_MAX_REPLY_LINES) {
            return -1;
        }
        line = read_line(input);
        if (line == NULL || parse_reply_code(line) != first_code) {
            free(line);
            return -1;
        }
        ++line_count;
    }

    *code = first_code;
    *final_line = line;
    return 0;
}

/* Handle short writes so protocol messages reach the transport in full. */
static int write_all(const session_transport *transport, const char *data,
                     size_t length)
{
    size_t written = 0U;
    while (written < length) {
        ssize_t count = transport->write(transport->context, data + written,
                                          length - written);
        if (count <= 0) {
            return -1;
        }
        written += (size_t) count;
    }
    return 0;
}

/* Build an envelope command without adding CRLF until it is sent. */
static char *build_argument_command(const char *prefix, const char *value,
                                    int envelope)
{
    int length = envelope ? snprintf(NULL, 0, "%s<%s>", prefix, value)
                          : snprintf(NULL, 0, "%s %s", prefix, value);
    if (length < 0) {
        return NULL;
    }

    char *raw = malloc((size_t) length + 1U);
    if (raw == NULL) {
        return NULL;
    }
    if (envelope) {
        (void) snprintf(raw, (size_t) length + 1U, "%s<%s>", prefix, value);
    } else {
        (void) snprintf(raw, (size_t) length + 1U, "%s %s", prefix, value);
    }
    return raw;
}

/* Include the received reply when reporting a failed SMTP exchange. */
static void report_reply_error(const char *operation, int expected, int actual,
                               const char *reply)
{
    if (reply != NULL) {
        fprintf(stderr, "%s: expected %d, received %d (%s)", operation,
                expected, actual, reply);
    } else {
        fprintf(stderr, "%s: expected %d, but no complete reply was received\n",
                operation, expected);
    }
}

/* Send one command, consume its reply, and enforce the expected status code. */
static int send_command(receiver *input, const char *operation,
                        const char *command, int expected)
{
    char *wire_command = build_command(command);
    if (wire_command == NULL) {
        fprintf(stderr, "%s: could not build command\n", operation);
        return -1;
    }

    int result = write_all(input->transport, wire_command, strlen(wire_command));
    free(wire_command);
    if (result < 0) {
        fprintf(stderr, "%s: transport write failed\n", operation);
        return -1;
    }

    int actual = -1;
    char *reply = NULL;
    result = read_reply(input, &actual, &reply);
    if (result < 0) {
        report_reply_error(operation, expected, actual, reply);
        free(reply);
        return -1;
    }
    if (actual != expected) {
        report_reply_error(operation, expected, actual, reply);
        free(reply);
        return -1;
    }
    free(reply);
    return 0;
}

/** Execute the complete SMTP conversation over the supplied transport. */
int session_run(const session_transport *transport, const char *from,
                const char *to, const char *subject, const char *body,
                const char *helo_host)
{
    if (transport == NULL || transport->read == NULL || transport->write == NULL ||
        from == NULL || to == NULL || subject == NULL || body == NULL ||
        helo_host == NULL) {
        return -1;
    }

    /* Keep unread bytes because reads may split or combine reply lines. */
    receiver input = {
        .transport = transport,
        .start = 0U,
        .end = 0U
    };
    int code = -1;
    char *reply = NULL;
    if (read_reply(&input, &code, &reply) < 0 || code != 220) {
        report_reply_error("greeting", 220, code, reply);
        free(reply);
        return -1;
    }
    free(reply);

    char *helo = build_argument_command("HELO", helo_host, 0);
    char *mail = build_argument_command("MAIL FROM:", from, 1);
    char *rcpt = build_argument_command("RCPT TO:", to, 1);
    char *payload = build_data_payload(from, to, subject, body);
    if (helo == NULL || mail == NULL || rcpt == NULL || payload == NULL) {
        fprintf(stderr, "session: could not build SMTP message\n");
        free(helo);
        free(mail);
        free(rcpt);
        free(payload);
        return -1;
    }

    /* Stop at the first failed SMTP step; later commands must not be sent. */
    int result = send_command(&input, "HELO", helo, 250);
    if (result == 0) {
        result = send_command(&input, "MAIL FROM", mail, 250);
    }
    if (result == 0) {
        result = send_command(&input, "RCPT TO", rcpt, 250);
    }
    if (result == 0) {
        result = send_command(&input, "DATA", "DATA", 354);
    }
    if (result == 0 && write_all(transport, payload, strlen(payload)) < 0) {
        fprintf(stderr, "message body: transport write failed\n");
        result = -1;
    }
    if (result == 0) {
        result = read_reply(&input, &code, &reply);
        if (result < 0 || code != 250) {
            report_reply_error("message body", 250, code, reply);
            result = -1;
        }
        free(reply);
        reply = NULL;
    }
    if (result == 0) {
        result = send_command(&input, "QUIT", "QUIT", 221);
    }

    free(helo);
    free(mail);
    free(rcpt);
    free(payload);
    return result;
}