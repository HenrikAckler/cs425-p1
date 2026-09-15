#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef TEST
#define main main_exclude
#endif

static void print_usage(const char *program)
{
    printf("Usage: %s -f <from> -t <to> [-s subject] [-b body] [-p port]\n"
           "          [-H helo-host] <server>\n\n"
           "  -f <from>       envelope sender\n"
           "  -t <to>         envelope recipient\n"
           "  -s <subject>    subject line (default: empty)\n"
           "  -b <body>       message body (default: read from stdin)\n"
           "  -p <port>       port or service name (default: 25)\n"
           "  -H <helo-host>  host name sent with HELO (default: localhost)\n"
           "  <server>        host name or address of the mail server\n",
           program);
}

static int contains_crlf(const char *value)
{
    return value != NULL && strpbrk(value, "\r\n") != NULL;
}

static char *read_stdin(void)
{
    size_t length = 0U;
    size_t capacity = 4096U;
    char *body = malloc(capacity);

    if (body == NULL) {
        return NULL;
    }

    while (!feof(stdin)) {
        size_t available = capacity - length - 1U;
        size_t count = fread(body + length, 1U, available, stdin);

        length += count;
        if (ferror(stdin) != 0) {
            free(body);
            return NULL;
        }
        if (count < available) {
            break;
        }
        if (capacity > (SIZE_MAX / 2U)) {
            free(body);
            return NULL;
        }

        capacity *= 2U;
        char *expanded = realloc(body, capacity);
        if (expanded == NULL) {
            free(body);
            return NULL;
        }
        body = expanded;
    }

    body[length] = '\0';
    return body;
}

int main(int argc, char **argv)
{
    const char *from = NULL;
    const char *to = NULL;
    const char *subject = "";
    const char *body_argument = NULL;
    const char *port = "25";
    const char *helo_host = "localhost";
    int option;

    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }

    opterr = 0;
    while ((option = getopt(argc, argv, "f:t:s:b:p:H:")) != -1) {
        switch (option) {
        case 'f':
            from = optarg;
            break;
        case 't':
            to = optarg;
            break;
        case 's':
            subject = optarg;
            break;
        case 'b':
            body_argument = optarg;
            break;
        case 'p':
            port = optarg;
            break;
        case 'H':
            helo_host = optarg;
            break;
        case '?':
        default:
            fprintf(stderr, "Invalid command-line option\n");
            print_usage(argv[0]);
            return 1;
        }
    }

    if (from == NULL || to == NULL || optind != argc - 1) {
        fprintf(stderr, "Options -f, -t, and one server argument are required\n");
        print_usage(argv[0]);
        return 1;
    }
    if (*from == '\0' || *to == '\0' || *port == '\0' || *helo_host == '\0') {
        fprintf(stderr, "Envelope addresses, port, and HELO host cannot be empty\n");
        return 1;
    }
    if (contains_crlf(from) || contains_crlf(to) || contains_crlf(subject)) {
        fprintf(stderr, "Envelope addresses and subject must not contain CR or LF\n");
        return 1;
    }

    char *body = body_argument == NULL ? read_stdin() : strdup(body_argument);
    if (body == NULL) {
        fprintf(stderr, "Unable to read the message body: %s\n", strerror(errno));
        return 2;
    }

    //Stub for session call
    (void) from;
    (void) to;
    (void) subject;
    (void) port;
    (void) helo_host;
    (void) argv[optind];
    free(body);

    fprintf(stderr, "Command-line parsing succeeded; SMTP transport is not implemented yet\n");
    return 0;
}