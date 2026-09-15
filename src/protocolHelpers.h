#ifndef PROTOCOL_HELPERS_H
#define PROTOCOL_HELPERS_H

int parse_reply_code(const char *line);
int reply_is_final(const char *line);
char *build_command(const char *command);
char *dot_stuff_body(const char *body);
char *build_data_payload(const char *from, const char *to,
                         const char *subject, const char *body);

#endif