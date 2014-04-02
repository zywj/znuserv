
#include <zs_core.h>

void
zs_init_timer(zs_context_t *ctx)
{
	timer = zs_palloc(ctx->pool, sizeof(zs_timer_t));
	timer->pool = zs_create_pool(1024);
	timer->rbt = zs_create_rbtree(timer);
}

void 
zs_add_timer_node(zs_context_t *ctx, zs_request_t *req, int time)
{
	zs_rb_node_t *node;

	node = zs_palloc(timer->pool, sizeof(zs_rb_node_t));
	node->key = zs_palloc(timer->pool, sizeof(struct zs_key_pair_s));
	node->key->req = req;
	node->key->data = time;

	zs_rb_insert(&timer->rbt->root, timer->rbt->sentinel, node);
}


void 
zs_del_timer_node(zs_context_t *ctx, zs_rb_node_t *node)
{
	zs_rb_delete(&timer->rbt->root, timer->rbt->sentinel, node);
}
