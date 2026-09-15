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