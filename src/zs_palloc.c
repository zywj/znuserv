
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 * 
 * Thanks for the good thinking of nginx.
 *
 */

#include <zs_core.h>

static void *zs_palloc_block(zs_pool_t *pool, size_t size);
static void *zs_palloc_large(zs_pool_t *pool, size_t size);

zs_pool_t *
zs_create_pool(size_t size)
{
    zs_pool_t *p;

    posix_memalign((void *)&p, (size_t)16, size);
    if (p == NULL) {
        return NULL; 
    }
    
    p->d.last = (u_char *)p + sizeof(zs_pool_t);
    p->d.end = (u_char *)p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = size - sizeof(zs_pool_t);
    p->max = (size < ZS_MAX_ALLOC) ? size : ZS_MAX_ALLOC;

    p->current = p;
    p->large = NULL;
    p->cleanup = NULL;
    
    return p;
}

void
zs_destroy_pool(zs_pool_t *pool)
{
    zs_pool_t *p, *n;
    zs_pool_large_t *l;
    zs_pool_cleanup_t *c;

    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) { 
            zs_err("run cleanup %p\n", c);
            c->handler(c->data);
        }
    }

    for (l = pool->large; l; l = l->next) {
        zs_err("large free %p\n", l);
        if (l->alloc)
            free(l->alloc);
    }

    for (p = pool, n = pool->d.next;  ; p = n, n = n->d.next) {
        free(p);
        zs_err("pool free %p\n", p);
        if (n == NULL)
            break;
    }
}

void
zs_reset_pool(zs_pool_t *pool)
{
    zs_pool_t        *p;
    zs_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    pool->large = NULL;

    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(zs_pool_t);
    }
}

void *
zs_palloc(zs_pool_t *pool, size_t size)
{
    u_char *m;
    zs_pool_t *p;

    if (size <= pool->max) {
        p = pool->current; 
        
        do {
            m = zs_align_ptr(p->d.last, ZS_ALIGNMENT);

            if ((size_t)(p->d.end - m) >= size) {
                p->d.last = m + size; 
                return m;
            } 
            p = p->d.next;
        
        } while(p);

        return zs_palloc_block(pool, size);
    }

    return zs_palloc_large(pool, size);
}

void *
zs_pnalloc(zs_pool_t *pool, size_t size)
{
    u_char *m;
    zs_pool_t *p;

    if (size <= pool->max) {
        p = pool->current; 
        
        do {
            m = p->d.last;

            if ((size_t)(p->d.end - m) >= size) {
                p->d.last = m + size; 
                return m;
            } 
            p = p->d.next;
        
        } while(p);

        return zs_palloc_block(pool, size);
    }

    return zs_palloc_large(pool, size);
}

static void *
zs_memalign(size_t alignment, size_t psize)
{
    int err;
    void *m;
    
    err = posix_memalign(&m, alignment, psize); 

    if (err) {
        zs_err("posix_memalign error.\n"); 
        m = NULL;
    }

    if (m == NULL )
        return NULL;

    return m;
}

void *
zs_palloc_block(zs_pool_t *pool, size_t size)
{
    u_char *m;
    size_t psize;
    zs_pool_t *p, *new, *current;

    psize = (size_t) (pool->d.end - (u_char *)pool);

    m = zs_memalign(ZS_POOL_ALIGN, psize);

    new = (zs_pool_t *) m;

    new->d.end = m + psize;
    new->d.next = NULL;
    new->d.failed = 0;

    m += sizeof(zs_pool_data_t);
    m = zs_align_ptr(m , ZS_ALIGNMENT);
    new->d.last = m + size;

    current = pool->current;

    for (p = current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) 
            current = p->d.next;
    }

    p->d.next = new;
    pool->current = current ? current : new;

    return m;
}

static void *
zs_palloc_large(zs_pool_t *pool, size_t size)
{
    void *p;
    uint_t n;
    zs_pool_large_t *large;

    p = malloc(size);
    if (p == NULL)
        return NULL;

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL)  {
            large->alloc = p; 
            return p;
        }

        if (n++ > 3)
            break;
    }

    large = zs_palloc(pool, sizeof(zs_pool_large_t));
    if (large == NULL){
        free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;
    
    return p;
}

int_t
zs_pfree(zs_pool_t *pool, void *p)
{
    zs_pool_large_t *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = NULL;

            return ZS_OK;
        }
    }

    return ZS_NO;
}

zs_pool_cleanup_t *
zs_pool_cleanup_add(zs_pool_t *p, size_t size)
{
    zs_pool_cleanup_t *c;

    c = zs_palloc(p, sizeof(zs_pool_cleanup_t));
    if (c == NULL)
        return NULL;

    if (size) {
        c->data = zs_palloc(p, size) ;
        if (c->data == NULL)
            return NULL;
    } else {
        c->data = NULL; 
    }

    c->handler = NULL;
    c->next = p->cleanup;
    p->cleanup = c;

    return c;
}
