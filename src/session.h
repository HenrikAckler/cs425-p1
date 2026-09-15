#ifndef SESSION_H
#define SESSION_H

#include <stddef.h>
#include <sys/types.h>

typedef ssize_t (*session_read_fn)(void *context, void *buffer, size_t length);
typedef ssize_t (*session_write_fn)(void *context, const void *buffer,
                                    size_t length);

typedef struct {
    session_read_fn read;
    session_write_fn write;
    void *context;
} session_transport;

int session_run(const session_transport *transport, const char *from,
                const char *to, const char *subject, const char *body,
                const char *helo_host);

#endif