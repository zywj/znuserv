
#include <zs_core.h>


int_t
zs_process_event(zs_context_t *ctx, int i)
{
	int_t nevents, k;
	sock_t listenfd;
	struct epoll_event elist[ZS_MAXEVENT]; 
	int epfd;
	struct epoll_event ee;
    zs_request_t *req;

	listenfd = ctx->listen_sock.sockfd;

	epfd = epoll_create(1);
	ctx->epfd = epfd;
	ctx->process_i = i;
	ctx->connection_num = 0;
	
	if (epfd < 0) {
		zs_err("epoll create failed.\n"); 
		return ZS_ERR;
	}

	req = zs_get_req(ctx, listenfd);
    req->sockfd = listenfd;

	ee.data.fd = listenfd;
	ee.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ee);

	do {
		nevents = epoll_wait(epfd, elist, ZS_MAXEVENT, -1);                
		if (nevents < 0) {
			zs_err("epoll wait failed.\n"); 
			return ZS_ERR;
		}
		
		//zs_err("[ $ ]: event num:%d\n", nevents);
		for (k = 0; k < nevents; k++) { 
			if ((elist[k].events & EPOLLERR)  ||
					(elist[k].events & EPOLLHUP)) {
				zs_err("epoll error.\n");

                close(elist[k].data.fd);

				continue;
			}

			zs_handle_request(ctx, elist[k].data.fd);
		}
		
	} while(1);
}
