
#ifndef _ZS_CACHE_H
#define _ZS_CACHE_H

#include <zs_core.h>

int_t zs_get_cache(zs_context_t *ctx, zs_request_t *req);
int_t zs_store_cache(zs_context_t *ctx, zs_request_t *req);
int_t zs_is_in_cache(zs_context_t *ctx, zs_request_t *req);

#endif