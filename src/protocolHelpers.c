#include "protocolHelpers.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Reject line breaks that could change SMTP command or header structure. */
int contains_crlf(const char *value)
{
    return value != NULL && strpbrk(value, "\r\n") != NULL;
}

/* Grow an output string while preserving its terminator and checking overflow. */
static int append_bytes(char **output, size_t *length, size_t *capacity,
                        const char *bytes, size_t count)
{
    if (count > SIZE_MAX - *length - 1U) { // GCOVR_EXCL_START
        return 0;
    }

    size_t required = *length + count + 1U;
    if (required > *capacity) {
        size_t new_capacity = *capacity;
        while (new_capacity < required) {
            if (new_capacity > SIZE_MAX / 2U) {
                new_capacity = required;
                break;
            }
            new_capacity *= 2U;
        }

        char *expanded = realloc(*output, new_capacity);
        if (expanded == NULL) {
            return 0;
        } // GCOVR_EXCL_STOP
        *output = expanded;
        *capacity = new_capacity;
    }

    memcpy(*output + *length, bytes, count);
    *length += count;
    (*output)[*length] = '\0';
    return 1;
}

/* Start the incrementally built strings used by body processing. */
static char *new_output(void)
{
    char *output = malloc(1U);
    if (output != NULL) { // GCOVR_EXCL_BR_LINE
        output[0] = '\0';
    }
    return output;
}

/** Parse the three-digit SMTP status code from a reply line. */
int parse_reply_code(const char *line)
{
    if (line == NULL || line[0] < '0' || line[0] > '9' ||
        line[1] < '0' || line[1] > '9' || line[2] < '0' || line[2] > '9' ||
        (line[3] != '-' && line[3] != ' ')) {
        return -1;
    }

    return (line[0] - '0') * 100 + (line[1] - '0') * 10 + (line[2] - '0');
}

/** Identify the final line of a possibly multi-line SMTP reply. */
int reply_is_final(const char *line)
{
    return parse_reply_code(line) >= 0 && line[3] == ' ';
}

/** Allocate an SMTP command with its required CRLF terminator. */
char *build_command(const char *command)
{
    if (command == NULL || contains_crlf(command)) {
        return NULL;
    }

    size_t command_length = strlen(command);
    if (command_length > SIZE_MAX - 3U) { // GCOVR_EXCL_START
        return NULL;
    }

    char *result = malloc(command_length + 3U);
    if (result == NULL) {
        return NULL;
    } // GCOVR_EXCL_STOP
    memcpy(result, command, command_length);
    memcpy(result + command_length, "\r\n", 3U);
    return result;
}

/** Normalize body lines and apply SMTP dot-stuffing. */
char *dot_stuff_body(const char *body)
{
    if (body == NULL) {
        return NULL;
    }

    char *output = new_output();
    if (output == NULL) { // GCOVR_EXCL_START
        return NULL;
    } // GCOVR_EXCL_STOP

    size_t length = 0U;
    size_t capacity = 1U;
    int at_line_start = 1;
    for (size_t index = 0U; body[index] != '\0'; ++index) {
        char current = body[index];
        if (current == '\r') {
            if (body[index + 1U] != '\n') {
                free(output);
                return NULL;
            }
                if (!append_bytes(&output, &length, &capacity, "\r\n", 2U)) { // GCOVR_EXCL_START
                free(output);
                return NULL;
                } // GCOVR_EXCL_STOP
            ++index;
            at_line_start = 1;
        } else if (current == '\n') {
            if (!append_bytes(&output, &length, &capacity, "\r\n", 2U)) { // GCOVR_EXCL_START
                free(output);
                return NULL;
            } // GCOVR_EXCL_STOP
            at_line_start = 1;
        } else {
            if (at_line_start && current == '.') {
                if (!append_bytes(&output, &length, &capacity, ".", 1U)) { // GCOVR_EXCL_START
                    free(output);
                    return NULL;
                } // GCOVR_EXCL_STOP
            }
            if (!append_bytes(&output, &length, &capacity, &current, 1U)) { // GCOVR_EXCL_START
                free(output);
                return NULL;
            } // GCOVR_EXCL_STOP
            at_line_start = 0;
        }
    }

    if (length > 0U && !at_line_start && // GCOVR_EXCL_BR_LINE
        !append_bytes(&output, &length, &capacity, "\r\n", 2U)) {
        free(output); // GCOVR_EXCL_LINE
        return NULL; // GCOVR_EXCL_LINE
    }
    return output;
}

/** Build the complete SMTP DATA payload, including its terminating period. */
char *build_data_payload(const char *from, const char *to,
                         const char *subject, const char *body)
{
    if (from == NULL || to == NULL || subject == NULL || body == NULL ||
        contains_crlf(from) || contains_crlf(to) || contains_crlf(subject)) {
        return NULL;
    }

    char *stuffed_body = dot_stuff_body(body);
    if (stuffed_body == NULL) {
        return NULL;
    }

    int header_length = snprintf(NULL, 0, "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n",
                                 from, to, subject);
    if (header_length < 0) { // GCOVR_EXCL_START
        free(stuffed_body);
        return NULL;
    } // GCOVR_EXCL_STOP

    size_t header_size = (size_t) header_length;
    size_t body_size = strlen(stuffed_body);
    if (header_size > SIZE_MAX - body_size - 4U) { // GCOVR_EXCL_START
        free(stuffed_body);
        return NULL;
    } // GCOVR_EXCL_STOP

    char *payload = malloc(header_size + body_size + 4U);
    if (payload == NULL) { // GCOVR_EXCL_START
        free(stuffed_body);
        return NULL;
    } // GCOVR_EXCL_STOP
    (void) snprintf(payload, header_size + 1U,
                    "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n",
                    from, to, subject);
    memcpy(payload + header_size, stuffed_body, body_size);
    memcpy(payload + header_size + body_size, ".\r\n\0", 4U);
    free(stuffed_body);
    return payload;
}