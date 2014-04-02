
#ifndef _ZS_RB_TIMER_H
#define _ZS_RB_TIMER_H

#include <zs_core.h>
 
extern zs_timer_t *timer;

struct zs_timer_s {
    int_t length;
    zs_pool_t *pool;
    
    zs_rbtree_t *rbt;
};

void zs_init_timer(zs_context_t *ctx);

void zs_add_timer_node(zs_context_t *ctx, zs_request_t *req, int time);

void zs_del_timer_node(zs_context_t *ctx, zs_rb_node_t *node);

#endif

