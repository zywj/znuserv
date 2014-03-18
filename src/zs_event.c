
#include <zs_core.h>


static char *rn = "\r\n";

static struct epoll_event ee;
static int_t epfd;

static int_t connection_num;
static int_t process_i;

static zs_request_t *req;
static zs_context_t *ctx;

static char *plain_suffix[] = {
    ".txt", 
    ".html",
    ".htm",
    ".js",
    ".css",
    NULL
};

static char *static_file_suffix[] = {
    ".jpg",
    ".jpeg",
    ".gif",
    ".png",
    ".ico",
    NULL
};

void
zs_cleanup()
{
	int n;
	int on = 0;
	setsockopt(req->sockfd, SOL_TCP, TCP_CORK, &on, sizeof(on));

	connection_num--;

	ee.data.ptr = req;
	n = epoll_ctl(epfd, EPOLL_CTL_DEL, req->sockfd, &ee);
	if (n < 0) {
		zs_err("epoll ctl del failed.\n");
	}

	zs_destroy_pool(req->pool);

	zs_err("[ Ed ] Process:%d sockfd:%d status:%d num:%d \n",
		process_i, req->sockfd, req->status, connection_num);

	close(req->sockfd);
	close(req->file_fd);
}

void 
zs_send_header(int v)
{
	int n, len;

	req->res_header = zs_palloc(req->pool, 256);

	switch (req->res_code) {
	
	case 404:
		req->res_header = ZS_404_header;
		break;

	case 400:
		req->res_header = ZS_400_header;
		break;

	case 200:
		req->res_header = ZS_200_header;
		break;
	}

	len = strlen(req->res_header);
	req->has_written = 0;

	while (len != req->has_written) {
		n = write(req->sockfd, req->res_header + req->has_written, len - req->has_written);

		if (n == -1) {
			if (errno == EAGAIN) {
				break;

			} else {
				return;
			}

		} else if (n == 0) {
			zs_err("sending header. Client close.\n");
			return ;

		} else if (n > 0) {
			req->has_written += n;
		}
	}

    if (v != 1) {
        req->status = ZS_WR_REQ;

    } else {
        req->status = ZS_SEND_STATIC; 
    }

	ee.events = EPOLLOUT | EPOLLET;
	ee.data.ptr = req;
	n = epoll_ctl(epfd, EPOLL_CTL_MOD, req->sockfd, &ee);

	if (n < 0) {
		zs_err("send header mod epoll error.\n");
	}
}

void 
zs_read_php()
{    
	int n;
	size_t size;

	size = 1 << 16;
	req->has_read = 0;
	zs_err("the reqeust: %s\n", req->pre->buf);
	while (1) {
		n = read(req->sockfd, req->pre->res_cnt + req->has_read, size - req->has_read);
		zs_err("read: %s\n", req->pre->res_cnt);
		if (n < 0) {
			if (errno == EAGAIN) {
				break;
			
			} else {
				zs_err("read php from apache error.\n");
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
	ee.events = EPOLLOUT | EPOLLET;
	ee.data.ptr = req->pre;
	n = epoll_ctl(epfd, EPOLL_CTL_MOD, req->pre->sockfd, &ee);

	if (n < 0) {
		zs_err("read php mod epoll error.\n");
	}

	n = epoll_ctl(epfd, EPOLL_CTL_DEL, req->sockfd, NULL);

	if (n < 0) {
		zs_err("read php del epoll error.\n");
	}

}

void
zs_send_php() 
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

	epoll_ctl(epfd, EPOLL_CTL_DEL, req->sockfd, NULL);

	close(req->sockfd);

	zs_destroy_pool(req->pool);
	connection_num--;
	zs_err("============== a php page has send ===============\n");
}

void 
zs_write_req_to_php()
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
	ee.events = EPOLLIN | EPOLLET;
	ee.data.ptr = req;
	n = epoll_ctl(epfd, EPOLL_CTL_MOD, req->sockfd, &ee); 

	if (n < 0) {
		zs_err("epoll add apache fd failed.\n");
	}
}

int_t 
zs_init_apache()
{
	int_t n, php_listen_port;
	zs_request_t *newreq, t;
	sock_t php_sockfd;
	struct sockaddr_in addr;

	req->is_php = 1;

	php_listen_port = ctx->conf->php_listen_port;

	php_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(php_listen_port);
	
	/*
	 * connection the sockfd 
	 */
	n = connect(php_sockfd, (struct sockaddr *) &addr, sizeof(addr));
	if (n == -1) {
		zs_err("connect to apache listen port failed.\n"); 

		return ZS_ERR;
	}
	
	zs_set_nonblocking(php_sockfd);

    t.pool = zs_create_pool(ZS_REQ_POOL_SIZE);
	newreq = &t;
	newreq->has_read = 0;
	newreq->has_written = 0;
	newreq->status = ZS_WR_REQ_PHP;
	newreq->sockfd = php_sockfd;
	newreq->pre = req;

    /* 
     * write event for write request to apache
     */
	ee.events = EPOLLOUT | EPOLLET;
	ee.data.ptr = newreq;
	n = epoll_ctl(epfd, EPOLL_CTL_ADD, php_sockfd, &ee);
    
    if (n < 0 ) {
        zs_err("write req to php epoll ctl error.\n"); 
        return ZS_ERR;
    }

	return ZS_OK;
}

void 
zs_get_request_line()
{
	int i;

	req->p = zs_palloc(req->pool, 512);
	i = strcspn(req->buf, rn);
	strncpy(req->p, req->buf, i);

	req->request_method = zs_palloc(req->pool, 10);
	req->uri = zs_palloc(req->pool, 492);
	req->http_version = zs_palloc(req->pool, 10);

	/*
	 * get the request line.
	 */
	sscanf(req->p, "%s %s %s", req->request_method, req->uri, req->http_version);
	req->http_version[8] = '\0';
	
	/*
	 * get the request file suffix
	 */
	req->suffix = strrchr(req->uri, '.');
    
    //zs_err("%s %s %s \n", req->request_method, req->uri, req->http_version);
}

int_t
zs_add_index_file() 
{
	int i, len, root_dir_len;
	FILE *f;

	root_dir_len = strlen(ctx->conf->root_dir);

	memcpy(req->pf, ctx->conf->root_dir, root_dir_len + 1); 
	req->pf[strlen(req->pf)] = '/';

	for (i = 0; i < 1; i++) { 
		len = strlen(ctx->conf->index_files[i]);

		/*
		 * trying to find the index file is exist
		 */
		memcpy(req->pf + 1 + root_dir_len, ctx->conf->index_files[i], len + 1);
		req->pf[strlen(req->pf)] = '\0';
		
		f = fopen(req->pf, "r"); 
		if (f > 0) {
			req->suffix = strrchr(req->pf, '.');
			return ZS_OK;
		} 
	} 

	if (f == NULL) {
		zs_err("index not found.\n");     
	}

	return ZS_NO;
}

int_t 
zs_is_plain(char *s)
{
    int i;

    for (i = 0; plain_suffix[i] != NULL; i++) {
        if (strcmp(s, plain_suffix[i]) == 0) 
            return ZS_OK;
    }

    return ZS_NO;
}

int_t 
zs_is_static_file(char *s)
{
    int i;

    for (i = 0; static_file_suffix[i] != NULL; i++) {
        if (strcmp(s, static_file_suffix[i]) == 0) 
            return ZS_OK;
    }

    return ZS_NO;
}

void 
zs_read_static_file()
{
    int n;
	
    fseek(req->fp, 0, SEEK_END);
    req->res_length = ftell(req->fp);
    rewind(req->fp);
    zs_err("length: %d\n", req->res_length);

    req->has_read = 0;
    req->res_fcnt = zs_palloc(req->pool, 500 * 1024);

    n =  fread(req->res_fcnt, 4096, req->res_length, req->fp);

    zs_err("n:%d\n", n);

    req->res_code = 200;

    ee.events = EPOLLOUT | EPOLLET;
    ee.data.ptr = req;
    req->status = ZS_SEND_STATIC;
    n = epoll_ctl(epfd, EPOLL_CTL_MOD, req->sockfd, &ee);

    if (n < 0) {
        zs_err("fread epoll ctl error\n");  
    }
}

void
zs_send_static_file()
{
    int n;
    
    zs_send_header(1);
    req->has_written = 0;
    if (req->res_fcnt != NULL) {
        while (req->has_written != req->res_length) {
            n = write(req->sockfd, req->res_fcnt + req->has_written, req->res_length - req->has_written); 

            zs_err("%d\n", n);
            if (n < 0) {
                if (errno == EAGAIN) {
                    break; 

                } else {
                    zs_err("fwrite error\n"); 
                    return;
                } 

            } else {
                req->has_written += n; 
            }
        }

    } else {
        zs_err("req->res_fcnt null\n"); 
        return;
    }

    zs_cleanup();
}

void
zs_process_static_file()
{
    req->fp = fopen(req->pf, "rb");
    if (req->fp != NULL) {
        zs_read_static_file();     
        zs_send_static_file();

    } else {
        zs_err("file not found\n"); 
    }
}

int_t 
zs_run_get_method()
{
	int n, len;
    zs_request_t t;
    struct stat f;

    t.pool = zs_create_pool(ZS_REQ_POOL_SIZE);

    req->pre = &t;
	req->pf = zs_palloc(req->pool, 1024);
	
	if (strlen(req->uri) == 1) {

		/*
		 * Add index file  
		 */
		zs_add_index_file(ctx, req); 

		if (req->pf == 0) {
			zs_err("req->pf is null.\n");
			return ZS_ERR;
		}

	} else {

		/*
		 * merge the root directory and the request string 
		 */
		len = strlen(ctx->conf->root_dir);
		memcpy(req->pf, ctx->conf->root_dir, len + 1);
		//req->pf[len] = '/';
		len = strlen(req->uri);
		memcpy(req->pf  + strlen(ctx->conf->root_dir), req->uri, len + 1);
		req->pf[strlen(req->pf)] = '\0';
	}

	/*
	 * if the request page is php file.
	 */
	if (req->suffix[strlen(req->suffix) - 1] == 'p') {
		if (zs_init_apache() != ZS_OK) {
			return ZS_ERR; 
		}

	} else if (zs_is_plain(req->suffix) == ZS_OK) {
		/*
		 * to know the request content is in cahce 
		 */
		n = zs_is_in_cache(ctx, req);
		if (n == ZS_OK) {
			req->res_code = 200;   

			/* 
			 * read event end, start write event 
			 */
			req->status = ZS_WR_HEADER;
			req->in_cache = 1;

			zs_get_cache(ctx, req);
			goto end;

		} else if (n == ZS_ERR) {
			return ZS_ERR;
		}

		req->file_fd = open(req->pf, O_RDONLY);
		if (req->file_fd == -1) {
			req->res_code = 404; 

		} else {
			req->res_code = 200;     
			req->res_cnt = zs_palloc(req->pool, (1 << 20));
			fstat(req->file_fd, &f);
			req->modified_time = f.st_mtime;

			zs_store_cache(ctx, req);

			/*
			 * read event end, start write event
			 */
			req->status = ZS_WR_HEADER;
		}

end:
		ee.events = EPOLLOUT | EPOLLET;
		ee.data.ptr = req;
		n = epoll_ctl(epfd, EPOLL_CTL_MOD, req->sockfd, &ee);

		if (n == -1) {
			zs_err("epoll modify error."); 
		}

		shutdown(req->sockfd, SHUT_RD);

	} else if (zs_is_static_file(req->suffix) == ZS_OK) {
        zs_process_static_file(); 
    }

	return ZS_OK;
}

void 
zs_parse_start()
{
	int n;
	
	/*
	 * if the request mehtod is GET
	 */
    n = zs_run_get_method(); 
    
    if (n == ZS_NO) {
        return; 
    }
}

void 
zs_parse_request()
{
	zs_get_request_line();

	zs_parse_start();	
}

void
zs_read_request()
{
	int_t n;

	req->buf = zs_palloc(req->pool, 1 << 20);

	while (1) {
		n = read(req->sockfd, req->buf + req->has_read, ZS_READ_MAX - req->has_read);         

		if (n == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				break;

			} else if (errno == ECONNRESET) {
				close(req->sockfd);
				req->sockfd = -1;

			} else {
				return;
			}

		} else if (n == 0) {
			return; 

		} else if (n > 0) {
			req->has_read += n; 
		}
	}
	req->buf[strlen(req->buf)] = '\0';

	//zs_err("======\nHEADER: %s\n======\n", req->buf);
	
	/*
	 * start to parse the request
	 */
	zs_parse_request();

}

void 
zs_send_cache_response() 
{
	int len, n;

	len = strlen(req->cache_buf);

	req->has_written = 0;
	while (len != req->has_written) {
		n = write(req->sockfd, req->cache_buf, len - req->has_written);

		if (n < 0) {
			if (errno == EAGAIN) {
				break;

			} else {
				zs_err("cache write error.\n");
				return ;
			}
		} else {
			req->has_written += n;
		}
	}

	zs_cleanup();

}

void
zs_send_response()
{
	int_t n;
	off_t offset;
	struct stat buf;

	fstat(req->file_fd, &buf);
	req->res_length = buf.st_size;  

	req->has_written = 0;
	while (1) {
		offset = req->has_written;
		n = sendfile(req->sockfd, req->file_fd, &offset, (1024 << 10) - req->has_written); 
		req->has_written = offset; 

		if (n < 0) {
			if (errno == EAGAIN) {
				break; 

			} else {
				return ; 
			}
		} 

		if (req->has_written == req->res_length) {
			zs_cleanup();

			return;
		}
	}  
}

void 
zs_handle_request()
{
	int_t n, on = 1;
	sock_t connfd, listenfd;
	zs_request_t *newreq ;
	struct sockaddr_in cliaddr;
	socklen_t socklen;

	listenfd = ctx->listen_sock.sockfd;    

	if (listenfd == req->sockfd) {

		/*
		 * the connection_num in this processes
		 * must less then the max num 
		 */
		if (connection_num < ctx->conf->worker_connections) {		
			while (1) {		

				/* 
				 * if the workers great than one , 
				 * then use file lock for only one process to 
				 * accept a new connection
				 */ 
				 /*
				if (ctx->conf->workers > 1) {
					if (flock(ctx->fd, LOCK_EX) < 0) {
						if (errno == EAGAIN || errno == EACCES) {

							
							 // if the file lock is held by another process, 
							 // then this porcess do nothing
							 
							zs_err("Process:%d has been locked!\n", i);
							return ;

						} else {
							zs_err("flock error\n");
							break;
						}
					}
				} */

				socklen = sizeof(cliaddr);
				connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);         

				if (connfd == -1) {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK))  {
						zs_err("has been accepted.\n");
						break; 

					} else if (errno == ECONNRESET){
						zs_err("Connction reset.\n");
						continue;

					} else {
						zs_err("accept error.\n");     
						break;
					}
				}
		
				connection_num++;

				n = zs_set_nonblocking(connfd);
				if  (n == -1) {
					return ; 
				}

				setsockopt(connfd, SOL_TCP, TCP_CORK, &on, sizeof(on));

				zs_err("[ Ac ] Process:%d Num:%d connfd:%d\n", 
                	process_i, connection_num, connfd);
				newreq = zs_palloc(ctx->pool, 1024);
				newreq->pool = zs_create_pool(1024);
				newreq->sockfd = connfd;
				newreq->status = ZS_RD_REQ;
				newreq->has_read = 0;
				newreq->has_written = 0;

				ee.data.ptr = newreq;
				ee.events = EPOLLIN | EPOLLET;
				n = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ee);

				if (n == -1) {
					zs_err("connfd epoll add failed.\n"); 
					return;
				}
				/*
				if (ctx->conf->workers > 1) {
					flock(ctx->fd, LOCK_UN);
				}*/
			}	
		}

	} else {	
		zs_err("[ Ev ] Process:%d sockfd:%d status:%d\n", 
			 process_i, req->sockfd, req->status);

		switch (req->status) {

		case ZS_RD_REQ:  
			zs_read_request();
			break;

		case ZS_WR_HEADER:
			zs_send_header(0);    
			break;

		case ZS_WR_REQ:
			if (req->in_cache != 1) {
				zs_send_response();

			} else {
				zs_send_cache_response();
			}

			break;

		case ZS_RD_PHP:
			zs_read_php();
			break;

		case ZS_WR_PHP:
			zs_send_php();
			break;

		case ZS_WR_REQ_PHP:
			zs_write_req_to_php();
			break;

        case ZS_SEND_STATIC:
            zs_send_static_file();
            break;
		}
	}
}

int_t
zs_process_event(zs_context_t *c, int i)
{
	int_t nevents, k;
	sock_t listenfd;
	struct epoll_event elist[ZS_MAXEVENT]; 
    zs_request_t t;

    process_i = i;
    ctx = c;
	listenfd = ctx->listen_sock.sockfd;

	epfd = epoll_create(1);
	
	if (epfd < 0) {
		zs_err("epoll create failed.\n"); 
		return ZS_ERR;
	}

    t.pool = zs_create_pool(ZS_REQ_POOL_SIZE);
    req = &t;
    req->sockfd = listenfd;

	ee.data.ptr = req;
	ee.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ee);

	do {
		nevents = epoll_wait(epfd, elist, ZS_MAXEVENT, -1);                
		if (nevents < 0) {
			zs_err("epoll wait failed.\n"); 
			return ZS_ERR;
		}
		
		zs_err("[ $ ]: event num:%d\n", nevents);
		for (k = 0; k < nevents; k++) { 
			if ((elist[k].events & EPOLLERR)  ||
					(elist[k].events & EPOLLHUP)) {
				zs_err("epoll error.\n");

                zs_request_t *r;
                r = (zs_request_t *)elist[k].data.ptr;
                close(r->sockfd);
                zs_destroy_pool(r->pool);

				continue;
			}

			req = (zs_request_t *)elist[k].data.ptr;

			zs_handle_request();
			zs_err("[ Fn ]: Process:%d sockfd:%d finish events:%d/%d\n\n",  process_i, req->sockfd, k + 1, nevents);
		}
		
	} while(1);
}
