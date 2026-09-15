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