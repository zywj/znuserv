
#ifndef _ZS_PROCESSES_H
#define _ZS_PROCESSES_H

#include <zs_core.h>

#define ZS_MAX_SOCKLEN 512
#define ZS_MAXEVENT 4096
#define ZS_MAX_READ 1024
#define ZS_MAX_PROCESSES 1024
#define ZS_MAX_REQS 1024


struct zs_process_s {
   	pid_t pid; 
   	int_t connection_num;

   	int_t epfd;
   	zs_pool_t *pool;

};


void zs_master_process(zs_context_t *ctx);
void zs_worker_process(zs_context_t *ctx);


#endif
