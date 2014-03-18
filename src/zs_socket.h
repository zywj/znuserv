
#ifndef _ZS_SOCKET_H
#define _ZS_SOCKET_H

#include <zs_core.h>

#define LISTENQ 4096
#define sock_t  int

union zs_usa_u {
    struct sockaddr sa;
    struct sockaddr_in sin;
};

struct zs_socket_s {
    int_t sockfd;
    union zs_usa_u local_sa;
    union zs_usa_u remote_sa;
};

struct zs_listen_s {
    sock_t fd;

};

int_t zs_set_nonblocking(sock_t s);
int_t zs_listen(zs_context_t *ctx);

#endif
