
#ifndef _ZS_CONTEXT_H
#define _ZS_CONTEXT_H


#include <zs_core.h>

#define Q_MAX_LEN 1024

struct zs_context_s {
    zs_pool_t *pool;
    pid_t pid;
    void *user_data;
    struct zs_socket_s listen_sock;
    zs_conf_t *conf;

    int fd;
    struct flock fl;
    lua_State *L;
};


void zs_context_init(zs_context_t *ctx);

#endif
