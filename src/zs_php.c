
#include <zs_core.h>

void 
zs_read_php(zs_context_t *ctx, zs_request_t *req)
{    
	int n;
	size_t size;

	size = 1 << 16;
	req->has_read = 0;
	req->pre->res_cnt = zs_palloc(req->pre->pool, 1 << 20);
	while (1) {
		n = read(req->sockfd, req->pre->res_cnt + req->has_read, size - req->has_read);

		if (n < 0) {
			if (errno == EAGAIN) {
				break;
			
			} else {
				perror("Read content from apache");
				return;
			} 

		} else if (n == 0) {
			zs_err("apache close the connection.\n");
			return ;

		} else if (n > 0) {
			req->has_read += n;
		}
	}

	req->pre->res_length = strlen(req->pre->res_cnt);
	req->pre->status = ZS_WR_PHP;
	ctx->ee.events = EPOLLOUT | EPOLLET;
	ctx->ee.data.ptr = req->pre;
	n = epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, req->pre->sockfd, &ctx->ee);

	if (n < 0) {
		zs_err("read php mod epoll error.\n");
	}

	n = epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, req->sockfd, NULL);

	if (n < 0) {
		zs_err("read php del epoll error.\n");
	}

}

void
zs_send_php(zs_context_t *ctx, zs_request_t *req) 
{
	int n;

	req->has_written = 0;

	while (req->res_length != req->has_written) {
		n = write(req->sockfd, req->res_cnt, req->res_length - req->has_written);

		if (n < 0) {
			if (errno == EAGAIN) {
				break;

			} else {
				return;
			}

		} else {
			req->has_written += n;
		}
	}

	epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, req->sockfd, NULL);

	close(req->sockfd);

	zs_destroy_pool(req->pool);
	ctx->connection_num--;
}

void 
zs_write_req_to_php(zs_context_t *ctx, zs_request_t *req)
{
	int len, n;

	/*
	 * send request to apache linsten port
	 */
	len = strlen(req->pre->buf);
	req->has_written = 0;

	while (len != req->has_written) {
	   n = write(req->sockfd, req->pre->buf + req->has_written, len - req->has_written); 

	   if (n < 0) {
			if (errno == EAGAIN) {
				break;

			} else {
				return  ;
			}

	   } else {
			req->has_written += n;
	   }
	}

	req->status = ZS_RD_PHP;
	ctx->ee.events = EPOLLIN | EPOLLET;
	ctx->ee.data.ptr = req;
	n = epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, req->sockfd, &ctx->ee); 

	if (n < 0) {
		zs_err("epoll add apache fd failed.\n");
	}
}

int_t 
zs_init_apache(zs_context_t *ctx, zs_request_t *req)
{
	int_t n, php_listen_port, on = 1;
	zs_request_t *newreq;
	sock_t php_sockfd;
	struct sockaddr_in addr;
 	struct hostent *hptr;

    req->pre = zs_palloc(ctx->pool, 1024);
    req->pre->pool = zs_create_pool(1024);
    
    //zs_err("server name is %s\n", ctx->conf->server_name);
    if ((hptr = gethostbyname(ctx->conf->server_name)) == NULL){
        zs_err("gethostbyname error.\n");
        return ZS_ERR;
    }

	req->is_php = 1;

	php_listen_port = ctx->conf->php_listen_port;

	php_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
    addr.sin_addr = *(struct in_addr *)hptr->h_addr;
	addr.sin_port = htons(php_listen_port);
	
	/*
	 * connection the sockfd 
	 */
	setsockopt(php_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	setsockopt(php_sockfd, SOL_SOCKET, TCP_CORK, &on, sizeof(on));
	n = connect(php_sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (n == -1) {
		zs_err("connect to apache listen port failed.\n"); 

		return ZS_ERR;
	}
	
	zs_set_nonblocking(php_sockfd);

	newreq = zs_get_req(ctx, php_sockfd);
	newreq->has_read = 0;
	newreq->has_written = 0;
	newreq->status = ZS_WR_REQ_PHP;
	newreq->sockfd = php_sockfd;
	newreq->pre = req;

    /* 
     * write event for write request to apache
     */
	ctx->ee.events = EPOLLOUT | EPOLLET;
	ctx->ee.data.ptr = newreq;
	n = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, php_sockfd, &ctx->ee);
    
    if (n < 0 ) {
        zs_err("write req to php epoll ctl error.\n"); 
        return ZS_ERR;
    }

	return ZS_OK;
}
