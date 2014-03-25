
#include <zs_core.h>


int_t
zs_process_event(zs_context_t *ctx, int i)
{
	int_t nevents, k;
	sock_t listenfd;
	struct epoll_event elist[ZS_MAXEVENT]; 
    zs_request_t *req;

	listenfd = ctx->listen_sock.sockfd;

	ctx->epfd = epoll_create(256);
	ctx->process_i = i;
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

	do {
		nevents = epoll_wait(ctx->epfd, elist, ZS_MAXEVENT, 400);                
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

			req = elist[k].data.ptr;

			zs_handle_request(ctx, req);
		}
		
	} while(1);
}
