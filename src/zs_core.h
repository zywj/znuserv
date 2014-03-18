
/*
 * This structure is nginx-like.
 */


#ifndef _ZS_CORE_H
#define _ZS_CORE_H

typedef struct zs_request_s       zs_request_t;
typedef struct zs_context_s       zs_context_t;
typedef struct zs_connection_s    zs_connection_t;
typedef struct zs_conf_s          zs_conf_t;
typedef struct zs_pool_s          zs_pool_t;
typedef struct zs_queue_s         zs_queue_t;
typedef struct zs_timer_s         zs_timer_t;
typedef struct zs_process_s       zs_process_t;
typedef unsigned                  uint_t;
typedef int                       int_t;
typedef int                       fd_t;


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include <lauxlib.h>
#include <lualib.h>

#include <zs_string.h>
#include <zs_config.h>
#include <zs_palloc.h>
#include <zs_socket.h>
#include <zs_context.h>
#include <zs_process.h>
#include <zs_timer.h>
#include <zs_event.h>
#include <zs_cache.h>


#define ZS_OK 1
#define ZS_NO 0
#define ZS_ERR -1


#define MAXFD 8


#endif

