
#include <zs_core.h>

zs_timer_t *timer;

void
zs_timeout(zs_context_t *ctx, zs_rb_node_t *node)
{
    zs_del_timer_node(ctx, node);
}

int_t
zs_process_event(zs_context_t *ctx, int pi)
{
	int_t nevents, k, timeout, i;
	sock_t listenfd;
	struct epoll_event elist[ZS_MAXEVENT]; 
	struct timeval now;
    zs_request_t *req;

	listenfd = ctx->listen_sock.sockfd;

	ctx->epfd = epoll_create(256);
	ctx->process_i = pi;
	ctx->connection_num = 0;
	
	if (ctx->epfd < 0) {
		zs_err("epoll create failed.\n"); 
		return ZS_ERR;
	}

	req = zs_get_req(ctx, listenfd);
    req->sockfd = listenfd;
	
	memset(&ctx->ee, 0, sizeof(ctx->ee));
	ctx->ee.events = EPOLLIN | EPOLLET;
	ctx->ee.data.ptr = req;
	epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, listenfd, &ctx->ee);

	zs_init_timer(ctx);

	do {

		// if event timeout is enabled
		if (ctx->conf->use_event_timeout == 1) {

            zs_rb_node_t *node;
			
			gettimeofday(&now, NULL);
			
			timeout = -1;
			if (timer->length > 0) {
				
                for (i = 0; i < timer->length; i++) {
				    node = zs_rbtree_min(timer->rbt->root, timer->rbt->sentinel);

                    if (node->key->data < now.tv_usec) {
						zs_timeout(ctx, node);
                        timeout = 0;
                        timer->length--;

					} else {
						timeout = node->key->data - now.tv_sec;
						break;
					}	
				} 
			}

		} else {
			timeout = -1;
		}

		nevents = epoll_wait(ctx->epfd, elist, ZS_MAXEVENT, timeout);                
		if (nevents < 0) {
			zs_err("epoll wait failed.\n"); 
			return ZS_ERR;
		}
		
		//zs_err("[ $ ]: event num:%d\n", nevents);
		for (k = 0; k < nevents; k++) { 
			if ((elist[k].events & EPOLLERR)  ||
					(elist[k].events & EPOLLHUP)) {

				req = elist[k].data.ptr;
				epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, req->sockfd, NULL);
				zs_err("sockfd:%d epoll error.\n", req->sockfd);
				break;
			}

			zs_handle_request(ctx, elist[k].data.ptr);
		}
		
	} while(1);
}
