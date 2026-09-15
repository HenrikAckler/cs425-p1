#ifndef SOCKET_TRANSPORT_H
#define SOCKET_TRANSPORT_H

#include "session.h"

typedef struct {
    int descriptor;
} socket_transport;

int socket_transport_connect(socket_transport *transport, const char *server,
                              const char *port);
void socket_transport_close(socket_transport *transport);
session_transport socket_transport_as_session(socket_transport *transport);

#endif