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