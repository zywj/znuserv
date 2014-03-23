
#ifndef _ZS_HTTP_H
#define _ZS_HTTP_H

#include <zs_core.h>


#define ZS_READ_MAX 1024
#define ZS_REQ_POOL_SIZE 1024
#define ZS_RD_REQ  0
#define ZS_WR_HEADER 1
#define ZS_WR_REQ 2
#define ZS_RD_PHP 3
#define ZS_WR_PHP 4
#define ZS_WR_REQ_PHP 5
#define ZS_SEND_STATIC 6


struct zs_request_s {
    zs_pool_t *pool;

    sock_t sockfd;  
    int_t file_fd;

    zs_request_t *pre;
    int_t status;    /* request status */
    int_t has_read;   /* the length of request has read */
    int_t has_written;  /* the length of request has written */

    char *buf;      /* request raw content */
    char *request_method;  
    char *uri;
    char *http_version;
    char *suffix;
    char *if_modified_since;
    int_t is_php:1;
    int_t res_length;   /* the length of the response content */

    char *pf;     /*  path + fullname */
    int_t in_cache;
    char *cache_buf;
    int_t is_static_file;

    char *res_header;
    FILE *fp;
    void *res_cnt;   /* the content of response */
    int_t res_code;   /* the response code */
    char *res_explain;  /*  */
    time_t res_date;
    time_t res_lastmod;
    char *res_lastmod_f;
    int modified_time;  /* for cache check*/

    char *p;
};

zs_request_t *zs_get_req(zs_context_t *ctx, sock_t sockfd);
void zs_handle_request(zs_context_t *ctx, sock_t sockfd);

#endif