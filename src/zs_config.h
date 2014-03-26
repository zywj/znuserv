
#ifndef _ZS_CONFIG_H
#define _ZS_CONFIG_H

#include <zs_core.h>

#define IDX_MAX 1024

#define zs_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


struct zs_conf_s {
    zs_pool_t *pool;
    int_t listen_port;
    char *server_name;
    char *index_files[IDX_MAX];
    char *root_dir;
    int_t workers;
    int_t worker_connections;
    int_t event_timeout;
    int_t php_listen_port;
    int_t is_deamon;
    int_t cache;
    int_t use_cache;
    char *page_404;
    char *pid;
};


int_t zs_get_config(zs_context_t *ctx);

#endif
