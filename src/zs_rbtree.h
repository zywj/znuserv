
#ifndef _ZS_RBTREE_H
#define _ZS_RBTREE_H

#include <zs_core.h>

typedef struct zs_rb_node_s  zs_rb_node_t;

struct zs_key_pair_s {
    zs_request_t *req;     
    long data;
};

struct zs_rb_node_s {
   enum {RED, BLACK} color;
   struct zs_key_pair_s *key;
   zs_rb_node_t *left;
   zs_rb_node_t *right;
   zs_rb_node_t *p;
};

struct zs_rbtree_s {
    zs_rb_node_t *root;
    zs_rb_node_t *sentinel;
};

void zs_rb_insert(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *z);

void zs_rb_delete(zs_rb_node_t **root, zs_rb_node_t *sentinel, zs_rb_node_t *z);

void zs_rbtree_traverse(zs_rb_node_t *node, zs_rb_node_t *sentinel);

zs_rb_node_t* zs_rbtree_search(zs_rb_node_t *node, zs_rb_node_t *sentinel, zs_rb_node_t *z);

zs_rbtree_t* zs_create_rbtree(zs_timer_t *t);

zs_rb_node_t * zs_rbtree_min(zs_rb_node_t *node, zs_rb_node_t *sentinel);

#endif

