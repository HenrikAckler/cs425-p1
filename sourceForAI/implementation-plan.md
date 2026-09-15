# SMTP Client Implementation Plan

This plan follows Tasks 1 and 2 of the assignment and keeps protocol logic separate from socket I/O so the session can be tested without a live server.

## 1. Define the Layer Headers

Replace the placeholder `lab.h` API with three layer-specific headers under `src/`:

- `protocolHelpers.h`
- `session.h`
- `socketTransport.h`

Define ownership and error conventions before implementation, including which functions return allocated strings, who frees them, and how callback errors are reported.

Use `protocolHelpers`, rather than the proposed `protocHelpers`, to correct the apparent spelling mistake.

## 2. Implement the Protocol Helpers

Implement `protocolHelpers.c` as the pure protocol layer. It must not perform any I/O.

Responsibilities:

- Parse a three-digit SMTP reply code.
- Identify continuation and final reply lines.
- Construct CRLF-terminated HELO, MAIL, RCPT, DATA, and QUIT commands.
- Dot-stuff each body line.
- Build the complete DATA payload with:
  - `From` header
  - `To` header
  - `Subject` header
  - Header/body separator
  - Dot-stuffed body
  - Final DATA terminator
- Reject or safely report malformed input.
- Reject CR/LF injection in address and subject data.

## 3. Implement the Session Layer

Implement `session.c` around a swappable transport interface containing read and write callbacks plus a context pointer.

Responsibilities:

- Read individual lines through the transport callbacks.
- Maintain a receive buffer so it handles:
  - Reads split across multiple callbacks.
  - Multiple replies in a single callback result.
  - CRLF line detection.
  - Oversized lines or replies.
  - Clean peer hangups.
- Read complete multiline replies.
- Write command and message data bytes.
- Send one command and verify its expected status code.
- Run the required SMTP sequence:

  `220 -> HELO/250 -> MAIL/250 -> RCPT/250 -> DATA/354 -> payload/250 -> QUIT/221`

Stop immediately after any transport failure or unexpected status code. Retain enough of the actual reply text to produce useful diagnostics.

The session layer owns SMTP sequencing. `main.c` should not duplicate this logic.

## 4. Implement the Socket Transport

Implement `socketTransport.c` as a thin real-transport adapter.

Responsibilities:

- Resolve the configured server and port with `getaddrinfo`.
- Try to establish a TCP connection with `connect`.
- Adapt `recv` and `send` to the session callbacks.
- Manage the socket file descriptor and cleanup.
- Report connection and system errors.

This layer must not understand SMTP commands, reply codes, or session sequencing.

## 5. Replace `main.c`

Replace the placeholder greeting program with the application orchestration layer.

Responsibilities:

- Use `getopt` for the required `-f`, `-t`, `-s`, `-b`, `-p`, and `-H` options.
- Accept the positional server argument.
- Apply the required defaults:
  - Subject: empty string
  - Body: read from stdin when `-b` is omitted
  - Port: `25`
  - HELO host: `localhost`
- Validate required values and reject CR/LF injection in address and subject input.
- Create and configure the socket transport.
- Invoke the session layer.
- Print useful errors, including the actual server reply when applicable.
- Return the required exit codes:
  - `0` for successful delivery and the no-argument usage path.
  - `1` for command-line errors.
  - `2` for connection or SMTP session failures.

## 6. Replace and Expand the Unity Tests

Replace the placeholder greeting tests with tests organized around the three public layer APIs.

Test every declared function and add a scripted in-memory transport for complete session tests.

Required coverage includes:

- Pure protocol helper behavior.
- Correct CRLF construction.
- Dot stuffing and DATA payload construction.
- Multiline replies.
- Replies split across multiple reads.
- Several replies in one read.
- Replies or lines larger than the receive buffer.
- A peer hangup in the middle of a session.
- Every unexpected status code in the required sequence.
- Malformed replies.
- Error-path cleanup and failure behavior.

The scripted transport must not use a network connection. It should allow tests to verify both the commands written by the session and the session's response to scripted server replies.

## Design Decisions

- Use `protocolHelpers.[h/c]`, not `protocHelpers.[h/c]`.
- Use three layer-specific headers and remove the placeholder `lab.h` API.
- Treat C modules as layers, using structs and function pointers for the swappable transport boundary.
- Keep `main.c` responsible for process and command-line concerns only.
- Keep SMTP sequencing in `session.c`.
- Keep the socket adapter unaware of SMTP protocol details.
- Keep multiple recipients, TLS, authentication, and `EHLO` out of scope because the assignment requires one recipient and `HELO` only.

