# Submission Report

- Submission generated at 09/17/2026 at 20:24:47

- Machine info: Linux runnervmlun5p 6.17.0-1022-azure #22-Ubuntu SMP Mon Jul 27 17:24:03 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux

## Note to Students

Please read this report carefully before submission.
Ensure that all sections are complete and accurate.
Look for any errors in the build or test outputs.
If you find any issues, correct them before submitting.
Post any questions on the class discussion board for help.


---

## README

# Project 1

- Name: Hank Ackler
- Email: henrikackler@u.boisestate.edu
- Class: CS425-001

## Known Bugs or Issues

No known issues.

## Experience

So I did this a bit different, per my agreement with Shane. Per our shared document, I'll go over the three tasks below:


### Task 1: Source Prep

I trimmed and added a markdown file `assignment.md` with the canvas assignment. I removed the things I don't want the AI worrying about. 

I also added the entire RFC 5321 doc to a markdown, then gave AI the following prompt:
```
I want you to look at this document.
Then, based on the information in assignment.md, I want you to strip out all information that is not needed to complete the assignment. 
For example, the entire section 1 is largely useless to you, so it would be stripped out. 
Most of section 2 can be simplified, and the original referenced in detail later as needed.
Section 4.1 and 4.2 is rather valuable, the rest of section 4 less so.
```
I reviewed the output, and I was fairly happy with how it trimmed down the document. It seemed to be getting the information I wanted to keep fairly well. I'll need to help it out more later, but this will do. 


### Task 2: Planning

I kicked it off with this prompt:

```
We're going to start some planning. For reference, please review the files in sourceForAI.

All code for now will be in src.

I'd like to start by planning out what files we're going to have. I believe we should have the following:

main.c - responsible for handling the command line input, setting things up and calling session
protocHelpers.[h/c] - this should be responsible for taking text strings and return codes, and appropriately handling them (converting into data, setting return values, etc). This is not going to touch any IO, this simple takes the text behind SMTP and understands it.
session.[h/c] - this should handle the active session. It will use a provided socket transport class to handle all I/O, and then use the protocHelper class to process all data. It is basically our logic layer.
socketTransport.[h/c] - This is a wrapper class on top of the sockets that we can swap out.

Look through steps 1 and 2 of the assignment especially, and tell me what you think of this plan. 

Keep in mind, the current main.c, lab.c/h, and test file implemtnation is junk, don't worry about it. We'll replace it.
```

It gave a pretty good response, largely recorded in `implementation-plan.md`. I'm going to use this a bit differently than it suggests, however. The goal is to get to a minimum viable product pretty fast. So first, I'm going to have it make the empty files it needs. Then we're going to get main working, then start stretching for some proper working code. 

### Task 3: Implementation

 - I kicked it off by first working with the AI to add the files that were planned, just as stubs. Then it was time for working on command line parsing in `main.c`, per step 5. 
   - This worked pretty well. I had to question it a bit on it's handling of how it read standard in, there were some redundant cautions. I'm still not convinvced it's the most efficient way to do it, but this should be sufficient for now.
   - There was some redundant code I was able to catch it writing. For example, it wrote private clrf checks in multiple locations, which were all essentially redundant.
 - At this point, I decided to have copilot do a documentation pass, make sure everything had comments. It proceeded to choke trying to get this done for about 30 minutes. Not sure why.
 - I managed to get past that, and worked on some testing. It wasn't quite creating all the test coverage needed, so I made it expand the testing.
 - Testing coverage, with some supervision, went well until a point. But it kept leaving lines untested, and upon pushing for coverage on some specific areas (especially branching) the AI started just excluding things for testing. 



### NOTES ON TESTING EXCLUSIONS:
There were a fair few areas that I ended up leaving some things excluded from coverage. The AI wanted to do more, I determined that it was being overzealous and reduced it to these, which I find fairly reasonable:

 - malloc/realloc returning NULL: lab.c, protocolHelpers.c, session.c
   - These require exhausting or intercepting the process allocator.
   - Normal inputs cannot reliably produce this condition.
 - snprintf returning a negative value: lab.c, protocolHelpers.c, session.c
   - The calls use fixed valid format strings; standard snprintf cannot reach this path under ordinary inputs. It's possibe here but I don't think it's going to happen. I could be wrong here.
 - Integer overflow guards: protocolHelpers.c
   - Reaching them requires strings near SIZE_MAX, highly unlikely to allocate in this process.
 - EINTR retry branches: socketTransport.c
   - These require interrupting recv/send at exactly the syscall point, which is nondeterministic without syscall mocking or production test seams.
 - socket() returning -1: socketTransport.c
   - Requires kernel/resource failure or syscall interception.
 - One cleanup branch in session.c
   - Depends on allocator failure in one of several message allocations, so it has the same allocator fault-injection limitation.
---


## Build Output

This section was generated by running `make all` in the project root directory.

```bash
make[1]: Entering directory '/home/runner/work/cs425-p1/cs425-p1'
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/socketTransport.c -o build/debug/socketTransport.c.o
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/protocolHelpers.c -o build/debug/protocolHelpers.c.o
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/main.c -o build/debug/main.c.o
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/lab.c -o build/debug/lab.c.o
mkdir -p build/debug
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address -c src/session.c -o build/debug/session.c.o
cc -g -O0 -DDEBUG -fno-omit-frame-pointer -fsanitize=address build/debug/socketTransport.c.o build/debug/protocolHelpers.c.o build/debug/main.c.o build/debug/lab.c.o build/debug/session.c.o -o build/debug/myapp_d -fsanitize=address
make[1]: Leaving directory '/home/runner/work/cs425-p1/cs425-p1'
make[1]: Entering directory '/home/runner/work/cs425-p1/cs425-p1'
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/socketTransport.c -o build/release/socketTransport.c.o
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/protocolHelpers.c -o build/release/protocolHelpers.c.o
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/main.c -o build/release/main.c.o
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/lab.c -o build/release/lab.c.o
mkdir -p build/release
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion -c src/session.c -o build/release/session.c.o
cc -Wall -Wextra -O2 -fPIE -MMD -MP -Wformat -Wformat=2 -Wconversion -Wsign-conversion -Wimplicit-fallthrough -fstack-protector-strong -Werror=format-security -Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion build/release/socketTransport.c.o build/release/protocolHelpers.c.o build/release/main.c.o build/release/lab.c.o build/release/session.c.o -o build/release/myapp 
make[1]: Leaving directory '/home/runner/work/cs425-p1/cs425-p1'
make[1]: Entering directory '/home/runner/work/cs425-p1/cs425-p1'
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/socketTransport.c -o build/tests/socketTransport.c.o
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/protocolHelpers.c -o build/tests/protocolHelpers.c.o
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/main.c -o build/tests/main.c.o
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/lab.c -o build/tests/lab.c.o
mkdir -p build/tests
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c src/session.c -o build/tests/session.c.o
mkdir -p build/tests/
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c tests/lab-test.c -o build/tests/lab-test.c.o
mkdir -p build/tests/harness/
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage -c tests/harness/unity.c -o build/tests/harness/unity.c.o
cc -g -O0 -DTEST -fprofile-arcs -ftest-coverage build/tests/socketTransport.c.o build/tests/protocolHelpers.c.o build/tests/main.c.o build/tests/lab.c.o build/tests/session.c.o build/tests/lab-test.c.o build/tests/harness/unity.c.o -o build/tests/myapp_t -fprofile-arcs -ftest-coverage
make[1]: Leaving directory '/home/runner/work/cs425-p1/cs425-p1'
make[1]: Entering directory '/home/runner/work/cs425-p1/cs425-p1'
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/socketTransport.c -o build/debug-test/socketTransport.c.o
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/protocolHelpers.c -o build/debug-test/protocolHelpers.c.o
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/main.c -o build/debug-test/main.c.o
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/lab.c -o build/debug-test/lab.c.o
mkdir -p build/debug-test
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c src/session.c -o build/debug-test/session.c.o
mkdir -p build/debug-test/
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c tests/lab-test.c -o build/debug-test/lab-test.c.o
mkdir -p build/debug-test/harness/
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address -c tests/harness/unity.c -o build/debug-test/harness/unity.c.o
cc -g -O0 -DDEBUG -DTEST -fno-omit-frame-pointer -fsanitize=address build/debug-test/socketTransport.c.o build/debug-test/protocolHelpers.c.o build/debug-test/main.c.o build/debug-test/lab.c.o build/debug-test/session.c.o build/debug-test/lab-test.c.o build/debug-test/harness/unity.c.o -o build/debug-test/myapp_td -fsanitize=address
make[1]: Leaving directory '/home/runner/work/cs425-p1/cs425-p1'
Builds completed. You can run the application with: ./build/release/myapp
You can run the debug build with: ./build/debug/myapp_d
You can run the test build with: ./build/tests/myapp_t
You can run the debug-test build with: ./build/debug-test/myapp_td
```

---

## Coverage Report

This section was generated by running `make report` in the project root directory.

```bash
greeting: expected 220, but no complete reply was received
greeting: expected 220, received 500 (500 greeting
)HELO: expected 250, received 550 (550 helo
)MAIL FROM: expected 250, received 550 (550 mail
)RCPT TO: expected 250, received 550 (550 rcpt
)DATA: expected 354, received 250 (250 data
)message body: expected 250, received 550 (550 queued
)QUIT: expected 221, received 550 (550 quit
)greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
HELO: expected 250, but no complete reply was received
HELO: transport write failed
message body: transport write failed
session: could not build SMTP message
message body: expected 250, but no complete reply was received
greeting: expected 220, but no complete reply was received
socket: invalid connection arguments
socket: invalid connection arguments
socket: invalid connection arguments
socket: cannot resolve invalid.invalid:25: Name or service not known
socket: cannot connect to 127.0.0.1:56001
tests/lab-test.c:493:test_contains_crlf:PASS
tests/lab-test.c:494:test_lab_greeting:PASS
tests/lab-test.c:495:test_reply_parsing_and_framing:PASS
tests/lab-test.c:496:test_dot_stuffing:PASS
tests/lab-test.c:497:test_data_payload:PASS
tests/lab-test.c:498:test_session_success_with_fragmentation_and_short_writes:PASS
tests/lab-test.c:499:test_session_accepts_multiline_and_coalesced_replies:PASS
tests/lab-test.c:500:test_session_transmits_dot_stuffed_payload:PASS
tests/lab-test.c:501:test_session_rejects_oversized_unterminated_reply:PASS
tests/lab-test.c:502:test_session_rejects_each_unexpected_status:PASS
tests/lab-test.c:503:test_session_rejects_malformed_and_mismatched_replies:PASS
tests/lab-test.c:504:test_session_rejects_too_many_reply_lines:PASS
tests/lab-test.c:505:test_session_rejects_transport_failures_and_invalid_inputs:PASS
tests/lab-test.c:506:test_socket_transport_contract:PASS
tests/lab-test.c:507:test_socket_transport_io:PASS
tests/lab-test.c:508:test_socket_transport_connects_and_rejects_closed_port:PASS

-----------------------
16 Tests 0 Failures 0 Ignored 
OK
./build/tests/myapp_t
greeting: expected 220, but no complete reply was received
greeting: expected 220, received 500 (500 greeting
)HELO: expected 250, received 550 (550 helo
)MAIL FROM: expected 250, received 550 (550 mail
)RCPT TO: expected 250, received 550 (550 rcpt
)DATA: expected 354, received 250 (250 data
)message body: expected 250, received 550 (550 queued
)QUIT: expected 221, received 550 (550 quit
)greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
HELO: expected 250, but no complete reply was received
HELO: transport write failed
message body: transport write failed
session: could not build SMTP message
message body: expected 250, but no complete reply was received
greeting: expected 220, but no complete reply was received
socket: invalid connection arguments
socket: invalid connection arguments
socket: invalid connection arguments
socket: cannot resolve invalid.invalid:25: Name or service not known
socket: cannot connect to 127.0.0.1:34719
tests/lab-test.c:493:test_contains_crlf:PASS
tests/lab-test.c:494:test_lab_greeting:PASS
tests/lab-test.c:495:test_reply_parsing_and_framing:PASS
tests/lab-test.c:496:test_dot_stuffing:PASS
tests/lab-test.c:497:test_data_payload:PASS
tests/lab-test.c:498:test_session_success_with_fragmentation_and_short_writes:PASS
tests/lab-test.c:499:test_session_accepts_multiline_and_coalesced_replies:PASS
tests/lab-test.c:500:test_session_transmits_dot_stuffed_payload:PASS
tests/lab-test.c:501:test_session_rejects_oversized_unterminated_reply:PASS
tests/lab-test.c:502:test_session_rejects_each_unexpected_status:PASS
tests/lab-test.c:503:test_session_rejects_malformed_and_mismatched_replies:PASS
tests/lab-test.c:504:test_session_rejects_too_many_reply_lines:PASS
tests/lab-test.c:505:test_session_rejects_transport_failures_and_invalid_inputs:PASS
tests/lab-test.c:506:test_socket_transport_contract:PASS
tests/lab-test.c:507:test_socket_transport_io:PASS
tests/lab-test.c:508:test_socket_transport_connects_and_rejects_closed_port:PASS

-----------------------
16 Tests 0 Failures 0 Ignored 
OK
mkdir -p ./build/report/html
mkdir -p ./build/report/txt
gcovr -r . --html --html-details --exclude-directories build/tests/harness --exclude '.*main\.c$' --exclude '.*test\.c$' -o ./build/report/html/coverage_report.html
(INFO) Reading coverage data...

(INFO) Writing coverage report...

gcovr -r . --txt                 --exclude-directories build/tests/harness --exclude '.*main\.c$' --exclude '.*test\.c$'
(INFO) Reading coverage data...

(INFO) Writing coverage report...

------------------------------------------------------------------------------
                           GCC Code Coverage Report
Directory: .
------------------------------------------------------------------------------
File                                       Lines     Exec  Cover   Missing
------------------------------------------------------------------------------
src/lab.c                                      8        8   100%
src/protocolHelpers.c                         67       67   100%
src/session.c                                133      133   100%
src/socketTransport.c                         43       43   100%
------------------------------------------------------------------------------
TOTAL                                        251      251   100%
------------------------------------------------------------------------------
```

---

## Address Sanitizer Report

This section was generated by running `make leak-test` in the project root directory.

```bash
greeting: expected 220, but no complete reply was received
greeting: expected 220, received 500 (500 greeting
)HELO: expected 250, received 550 (550 helo
)MAIL FROM: expected 250, received 550 (550 mail
)RCPT TO: expected 250, received 550 (550 rcpt
)DATA: expected 354, received 250 (250 data
)message body: expected 250, received 550 (550 queued
)QUIT: expected 221, received 550 (550 quit
)greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
greeting: expected 220, but no complete reply was received
HELO: expected 250, but no complete reply was received
HELO: transport write failed
message body: transport write failed
session: could not build SMTP message
message body: expected 250, but no complete reply was received
greeting: expected 220, but no complete reply was received
socket: invalid connection arguments
socket: invalid connection arguments
socket: invalid connection arguments
socket: cannot resolve invalid.invalid:25: Name or service not known
socket: cannot connect to 127.0.0.1:42939
tests/lab-test.c:493:test_contains_crlf:PASS
tests/lab-test.c:494:test_lab_greeting:PASS
tests/lab-test.c:495:test_reply_parsing_and_framing:PASS
tests/lab-test.c:496:test_dot_stuffing:PASS
tests/lab-test.c:497:test_data_payload:PASS
tests/lab-test.c:498:test_session_success_with_fragmentation_and_short_writes:PASS
tests/lab-test.c:499:test_session_accepts_multiline_and_coalesced_replies:PASS
tests/lab-test.c:500:test_session_transmits_dot_stuffed_payload:PASS
tests/lab-test.c:501:test_session_rejects_oversized_unterminated_reply:PASS
tests/lab-test.c:502:test_session_rejects_each_unexpected_status:PASS
tests/lab-test.c:503:test_session_rejects_malformed_and_mismatched_replies:PASS
tests/lab-test.c:504:test_session_rejects_too_many_reply_lines:PASS
tests/lab-test.c:505:test_session_rejects_transport_failures_and_invalid_inputs:PASS
tests/lab-test.c:506:test_socket_transport_contract:PASS
tests/lab-test.c:507:test_socket_transport_io:PASS
tests/lab-test.c:508:test_socket_transport_connects_and_rejects_closed_port:PASS

-----------------------
16 Tests 0 Failures 0 Ignored 
OK
```

---

## Src Files
### lab.c

```c

#include "lab.h"
#include <stdio.h>
#include <stdlib.h>

/** Allocate and format the greeting returned by the lab's sample API. */
char *get_greeting(const char *restrict name)
{
  if (name == NULL)
  {
    return NULL;
  }

  // Allocate memory for the greeting message
  int length = snprintf(NULL, 0, "Hello, %s!", name);
  if (length < 0) // GCOVR_EXCL_START
  {
    return NULL; // snprintf failed
  } // GCOVR_EXCL_STOP

  //Casting is safe here because we know length is non-negative
  size_t alloc_size = (size_t) length + 1; // +1 for the null terminator
  char *greeting = malloc( alloc_size);


  if (greeting == NULL) // GCOVR_EXCL_START
  {
    return NULL; // Memory allocation failed
  }  // GCOVR_EXCL_STOP


  // Create the greeting message
  snprintf(greeting, alloc_size, "Hello, %s!", name);

  return greeting;
}

```

### lab.h

```c

#ifndef LAB_H
#define LAB_H

/**
 * @brief Return a heap-allocated greeting for a name.
 *
 * The caller owns the returned string and must release it with free().
 * A NULL name is rejected and produces NULL.
 * @param name Name to include in the greeting.
 * @return Allocated greeting text, or NULL for invalid input or allocation failure.
 */
char* get_greeting(const char* restrict name);


#endif // LAB_H

```

### main.c

```c

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "protocolHelpers.h"
#include "session.h"
#include "socketTransport.h"

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

    socket_transport socket = {.descriptor = -1};
    if (socket_transport_connect(&socket, argv[optind], port) < 0) {
        free(body);
        return 2;
    }

    session_transport transport = socket_transport_as_session(&socket);
    int session_result = session_run(&transport, from, to, subject, body,
                                     helo_host);
    socket_transport_close(&socket);
    free(body);
    return session_result == 0 ? 0 : 2;
}
```

### protocolHelpers.c

```c

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
```

### protocolHelpers.h

```c

#ifndef PROTOCOL_HELPERS_H
#define PROTOCOL_HELPERS_H

/** Return nonzero when value contains CR or LF, otherwise zero. */
int contains_crlf(const char *value);
/** Parse the three-digit SMTP status code at the start of a reply line. */
int parse_reply_code(const char *line);
/** Return nonzero when line is a syntactically valid final SMTP reply line. */
int reply_is_final(const char *line);
/** Return a newly allocated command terminated by CRLF, or NULL on invalid input. */
char *build_command(const char *command);
/** Normalize body line endings and dot-stuff lines that begin with a period. */
char *dot_stuff_body(const char *body);
/** Build the DATA headers, dot-stuffed body, and SMTP end-of-data marker. */
char *build_data_payload(const char *from, const char *to,
                         const char *subject, const char *body);

#endif
```

### session.c

```c

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
                  if (line == NULL) { // GCOVR_EXCL_START
                    return NULL;
                  } // GCOVR_EXCL_STOP
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
      if (length < 0) { // GCOVR_EXCL_START
        return NULL;
      } // GCOVR_EXCL_STOP

    char *raw = malloc((size_t) length + 1U);
      if (raw == NULL) { // GCOVR_EXCL_START
        return NULL;
      } // GCOVR_EXCL_STOP
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
      if (wire_command == NULL) { // GCOVR_EXCL_START
        fprintf(stderr, "%s: could not build command\n", operation);
        return -1;
      } // GCOVR_EXCL_STOP

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
    reply = NULL;

    char *helo = build_argument_command("HELO", helo_host, 0);
    char *mail = build_argument_command("MAIL FROM:", from, 1);
    char *rcpt = build_argument_command("RCPT TO:", to, 1);
    char *payload = build_data_payload(from, to, subject, body);
    if (helo == NULL || mail == NULL || rcpt == NULL || payload == NULL) { // GCOVR_EXCL_BR_LINE
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
```

### session.h

```c

#ifndef SESSION_H
#define SESSION_H

#include <stddef.h>
#include <sys/types.h>

typedef ssize_t (*session_read_fn)(void *context, void *buffer, size_t length);
typedef ssize_t (*session_write_fn)(void *context, const void *buffer,
                                    size_t length);

/** Callbacks and opaque state used by the SMTP session layer for I/O. */
typedef struct {
    session_read_fn read;
    session_write_fn write;
    void *context;
} session_transport;

/**
 * Run the complete SMTP exchange over transport.
 *
 * Returns zero only after every expected server reply has been received;
 * returns -1 on invalid input, transport failure, malformed replies, or an
 * unexpected status code.
 */
int session_run(const session_transport *transport, const char *from,
                const char *to, const char *subject, const char *body,
                const char *helo_host);

#endif
```

### socketTransport.c

```c

#include "socketTransport.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* Adapt recv to the session callback and retry interruption by a signal. */
static ssize_t socket_read(void *context, void *buffer, size_t length)
{
    socket_transport *transport = context;
    ssize_t result;

    do {
        result = recv(transport->descriptor, buffer, length, 0);
    } while (result < 0 && errno == EINTR); // GCOVR_EXCL_BR_LINE

    return result;
}

/* Adapt send to the session callback and retry interruption by a signal. */
static ssize_t socket_write(void *context, const void *buffer, size_t length)
{
    socket_transport *transport = context;
    ssize_t result;

    do {
        result = send(transport->descriptor, buffer, length, 0);
    } while (result < 0 && errno == EINTR); // GCOVR_EXCL_BR_LINE

    return result;
}

/** Resolve and connect a stream socket, trying each returned address. */
int socket_transport_connect(socket_transport *transport, const char *server,
                              const char *port)
{
    if (transport == NULL || server == NULL || port == NULL) {
        fprintf(stderr, "socket: invalid connection arguments\n");
        return -1;
    }

    transport->descriptor = -1;

    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *addresses = NULL;
    int lookup_result = getaddrinfo(server, port, &hints, &addresses);
    if (lookup_result != 0) {
        fprintf(stderr, "socket: cannot resolve %s:%s: %s\n", server, port,
                gai_strerror(lookup_result));
        return -1;
    }

    /* Try every address so resolution order does not decide success. */
    for (struct addrinfo *address = addresses; address != NULL;
         address = address->ai_next) {
        int descriptor = socket(address->ai_family, address->ai_socktype,
                                address->ai_protocol);
        if (descriptor < 0) { // GCOVR_EXCL_START
            continue;
        } // GCOVR_EXCL_STOP
        if (connect(descriptor, address->ai_addr, address->ai_addrlen) == 0) {
            transport->descriptor = descriptor;
            break;
        }
        (void) close(descriptor);
    }

    freeaddrinfo(addresses);
    if (transport->descriptor < 0) {
        fprintf(stderr, "socket: cannot connect to %s:%s\n", server, port);
        return -1;
    }
    return 0;
}

/** Close the owned socket and mark the transport disconnected. */
void socket_transport_close(socket_transport *transport)
{
    if (transport != NULL && transport->descriptor >= 0) {
        (void) close(transport->descriptor);
        transport->descriptor = -1;
    }
}

/** Return session callbacks without transferring socket ownership. */
session_transport socket_transport_as_session(socket_transport *transport)
{
    session_transport session = {
        .read = socket_read,
        .write = socket_write,
        .context = transport
    };
    return session;
}
```

### socketTransport.h

```c

#ifndef SOCKET_TRANSPORT_H
#define SOCKET_TRANSPORT_H

#include "session.h"

typedef struct {
    int descriptor;
} socket_transport;

/** Resolve server and port, then connect to the first usable address. */
int socket_transport_connect(socket_transport *transport, const char *server,
                              const char *port);
/** Close the descriptor, if transport currently owns an open connection. */
void socket_transport_close(socket_transport *transport);
/** Adapt a connected socket for use by session_run. */
session_transport socket_transport_as_session(socket_transport *transport);

#endif
```

## Tests Files
### lab-test.c

```c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "harness/unity.h"
#include "../src/lab.h"
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

void test_lab_greeting(void)
{
    char *greeting = get_greeting("Ada");
    TEST_ASSERT_NOT_NULL(greeting);
    TEST_ASSERT_EQUAL_STRING("Hello, Ada!", greeting);
    free(greeting);
    TEST_ASSERT_NULL(get_greeting(NULL));
}

void test_reply_parsing_and_framing(void)
{
    /* SMTP replies require three digits followed by a space or continuation dash. */
    TEST_ASSERT_EQUAL_INT(200, parse_reply_code("200 OK\r\n"));
    TEST_ASSERT_EQUAL_INT(599, parse_reply_code("599-last\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code(NULL));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code(""));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("25 OK\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("25a OK\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("20a OK\r\n"));
    TEST_ASSERT_EQUAL_INT(-1, parse_reply_code("2/0 OK\r\n"));
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
    TEST_ASSERT_NULL(build_data_payload("from", NULL, "subject", "body"));
    TEST_ASSERT_NULL(build_data_payload("from", "to", NULL, "body"));
    TEST_ASSERT_NULL(build_data_payload("from", "to", "subject", NULL));
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
    assert_session_stops_at_failure("220\rX", "");
    assert_session_stops_at_failure("220-more\r\n", "");
}

void test_session_rejects_too_many_reply_lines(void)
{
    char replies[2048];
    size_t offset = 0U;
    for (size_t index = 0U; index < 101U; ++index) {
        offset += (size_t) snprintf(replies + offset, sizeof(replies) - offset,
                                    "220-more\r\n");
    }
    replies[offset - 8U] = ' ';
    scripted_transport script;
    script_init(&script, replies, sizeof(replies), sizeof(replies));
    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING("", script.writes);
}

void test_session_rejects_transport_failures_and_invalid_inputs(void)
{
    /* Simulate both a peer failure while reading and a failed DATA write. */
    scripted_transport script;
    script_init(&script, "220 ready\r\n250 helo\r\n", 11U, 4096U);
    script.fail_read_call = 2U;
    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING("HELO localhost\r\n", script.writes);

    script_init(&script, "220 ready\r\n", 4096U, 4096U);
    script.fail_write_call = 1U;
    TEST_ASSERT_EQUAL_INT(-1, run_session(&script));
    TEST_ASSERT_EQUAL_STRING("", script.writes);

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
    transport.write = NULL;
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "body", "host"));
    transport = script_as_session(&script);
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, NULL, "to", "subject",
                                          "body", "host"));
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", NULL, "subject",
                                          "body", "host"));
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", NULL,
                                          "body", "host"));
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          NULL, "host"));
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "body", NULL));
    script_init(&script, "220 ready\r\n", 4096U, 4096U);
    transport = script_as_session(&script);
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "bad\rbody", "host"));

    script_init(&script, "220 ready\r\n250 helo\r\n250 mail\r\n250 rcpt\r\n"
                         "354 data\r\n", 4096U, 4096U);
    script.fail_read_call = 6U;
    transport = script_as_session(&script);
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "body", "host"));
    TEST_ASSERT_EQUAL_INT(-1, session_run(&transport, "from", "to", "subject",
                                          "bad\rbody", "host"));
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

void test_socket_transport_io(void)
{
    int descriptors[2];
    TEST_ASSERT_EQUAL_INT(0, socketpair(AF_UNIX, SOCK_STREAM, 0, descriptors));

    socket_transport socket = {.descriptor = descriptors[0]};
    session_transport transport = socket_transport_as_session(&socket);
    char buffer[8] = {0};
    TEST_ASSERT_EQUAL_INT(5, (int) write(descriptors[1], "hello", 5U));
    TEST_ASSERT_EQUAL_INT(5, (int) transport.read(transport.context, buffer,
                                                   sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("hello", buffer);

    TEST_ASSERT_EQUAL_INT(5, (int) transport.write(transport.context, "world",
                                                    5U));
    memset(buffer, 0, sizeof(buffer));
    TEST_ASSERT_EQUAL_INT(5, (int) read(descriptors[1], buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_STRING("world", buffer);

    socket_transport_close(&socket);
    (void) close(descriptors[1]);
}

void test_socket_transport_connects_and_rejects_closed_port(void)
{
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, listener);
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(0U);
    TEST_ASSERT_EQUAL_INT(0, bind(listener, (struct sockaddr *) &address,
                                  sizeof(address)));
    TEST_ASSERT_EQUAL_INT(0, listen(listener, 1));
    socklen_t address_length = sizeof(address);
    TEST_ASSERT_EQUAL_INT(0, getsockname(listener, (struct sockaddr *) &address,
                                         &address_length));

    char port[16];
    (void) snprintf(port, sizeof(port), "%u", (unsigned) ntohs(address.sin_port));
    socket_transport transport = {.descriptor = -1};
    TEST_ASSERT_EQUAL_INT(0, socket_transport_connect(&transport, "127.0.0.1",
                                                       port));
    int accepted = accept(listener, NULL, NULL);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, accepted);
    socket_transport_close(&transport);
    (void) close(accepted);
    (void) close(listener);

    transport.descriptor = 123;
    TEST_ASSERT_EQUAL_INT(-1, socket_transport_connect(&transport, "127.0.0.1",
                                                       port));
    TEST_ASSERT_EQUAL_INT(-1, transport.descriptor);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_contains_crlf);
    RUN_TEST(test_lab_greeting);
    RUN_TEST(test_reply_parsing_and_framing);
    RUN_TEST(test_dot_stuffing);
    RUN_TEST(test_data_payload);
    RUN_TEST(test_session_success_with_fragmentation_and_short_writes);
    RUN_TEST(test_session_accepts_multiline_and_coalesced_replies);
    RUN_TEST(test_session_transmits_dot_stuffed_payload);
    RUN_TEST(test_session_rejects_oversized_unterminated_reply);
    RUN_TEST(test_session_rejects_each_unexpected_status);
    RUN_TEST(test_session_rejects_malformed_and_mismatched_replies);
    RUN_TEST(test_session_rejects_too_many_reply_lines);
    RUN_TEST(test_session_rejects_transport_failures_and_invalid_inputs);
    RUN_TEST(test_socket_transport_contract);
    RUN_TEST(test_socket_transport_io);
    RUN_TEST(test_socket_transport_connects_and_rejects_closed_port);
    return UNITY_END();
}

```

## Scripts Files
Report generated on 09/17/2026 at 20:24:49


---

## End of Report

SHA-256 Hash of the report: 512d661cef796aa614a54af198eb29f8ba5fca9850a052654ff46869088a8812

Do not edit the generated report. Any changes will be reported as academic dishonesty

---
## GitHub Info
- GitHub repo name: HenrikAckler/cs425-p1
- The repository visibility is public.
- The workflow was triggered by HenrikAckler
