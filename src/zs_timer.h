
#ifndef _ZS_TIMER_H
#define _ZS_TIMER_H

#include <zs_core.h>

#define PARENT(i) (i)/2
#define LEFT(i) (i)*2
#define RIGHT(i) (i)*2+1


typedef int_t (*timer_node_insert_pt)(time_t t);

struct zs_timer_node_s {
    void *data;
    time_t time;
};

struct zs_timer_s {
    int_t length;
    struct zs_timer_node_s  *node;
    zs_pool_t *pool;
};

int_t zs_timer_init();

int_t zs_timer_add_node(time_t time);
int_t zs_timer_del_node();

inline struct zs_timer_node_s zs_timer_del_min();
inline struct zs_timer_node_s zs_timer_get_min();

#endif

