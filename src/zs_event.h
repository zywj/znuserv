
#ifndef _ZS_EVENT_H
#define _ZS_EVENT_H

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


#define ZS_404_header "HTTP/1.1 404 Not found\r\nServer: znuserv\r\nContent_Type: text/html\r\nConnection: close\r\n\r\n"
#define ZS_400_header "HTTP/1.1 400 Bad request\r\nServer: znuserv\r\nContent_Type: text/html\r\nConnection: close\r\n\r\n"
#define ZS_200_header "HTTP/1.1 200 OK\r\nServer: znuserv\r\nContent_Type: text/html\r\nConnection: close\r\n\r\n"

#define ZS_404_CNT "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not found.</h1></center><hr><center>znuerser </center></body></html>"
#define ZS_400_CNT "<html><head><title>400 Bad Reqest</title></head><body><center><h1>400 Bad Request.</h1></center><hr><center>znuerser </center></body></html>"


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
    int modified_time;

    char *p;
};

int_t zs_process_event(zs_context_t *ctx, int i);

#endif

