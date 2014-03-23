
/* 
 * Copyright (C) 2002-2013 Igor Sysoev
 * Copyright (C) 2011-2013 Nginx, Inc.
 * All rights reserved.
 * 
 * Thanks for the good thinking of nginx.
 */


#ifndef _ZS_PALLOC_H
#define _ZS_PALLOC_H

#include <zs_core.h>

#define ZS_ALIGNMENT  sizeof(unsigned long)
#define ZS_MAX_POOL (1024 * 16)
#define ZS_MAX_ALLOC 2048
#define ZS_POOL_ALIGN 16

#define zs_align_ptr(p, a)  (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


typedef void (*zs_pool_cleanup_pt)(void *data);

typedef struct zs_pool_cleanup_s zs_pool_cleanup_t;
typedef struct zs_pool_large_s zs_pool_large_t;


struct zs_pool_cleanup_s {
    zs_pool_cleanup_pt handler;
    void *data;
    zs_pool_cleanup_t *next;
};


struct zs_pool_large_s {
    zs_pool_large_t *next;
    void *alloc;
};


typedef struct {
    u_char *last;
    u_char *end;
    zs_pool_t *next;    
    int_t failed;
}zs_pool_data_t;


struct zs_pool_s {
    zs_pool_data_t d;
    size_t max;
    zs_pool_t *current;
    zs_pool_large_t *large;
    zs_pool_cleanup_t *cleanup;
};


zs_pool_t *zs_create_pool(size_t size);
void zs_destroy_pool(zs_pool_t *pool);

void *zs_palloc(zs_pool_t *pool, size_t size);

zs_pool_cleanup_t *zs_pool_cleanup_add(zs_pool_t *p, size_t size);
int_t zs_pfree(zs_pool_t *pool, void *p);
void zs_reset_pool(zs_pool_t *pool);
#endif
