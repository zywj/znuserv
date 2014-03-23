
#ifndef _ZS_PHP_H
#define _ZS_PHP_H

#include <zs_core.h>

void zs_read_php(zs_context_t *ctx, zs_request_t *req);
void zs_send_php(zs_context_t *ctx, zs_request_t *req);
void zs_write_req_to_php(zs_context_t *ctx, zs_request_t *req);
int_t zs_init_apache(zs_context_t *ctx, zs_request_t *req);

#endif