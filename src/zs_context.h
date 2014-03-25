
#ifndef _ZS_CONTEXT_H
#define _ZS_CONTEXT_H


#include <zs_core.h>

#define Q_MAX_LEN 1024

struct zs_context_s {
    zs_pool_t *pool;
    pid_t pid;
    struct zs_socket_s listen_sock;
    zs_conf_t *conf;

    // request pool
    zs_request_t *reqs;  
    zs_request_t *free_reqs;

    int epfd;
    struct epoll_event ee;
    int_t connection_num;
    int_t process_i;

    int fd;
    struct flock fl;
    lua_State *L;
    lua_State *Hdr;
};


void zs_context_init(zs_context_t *ctx);

#endif
