
#include <zs_core.h>

static char *rn = "\r\n";

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

zs_request_t *
zs_get_req(zs_context_t *ctx, int sockfd)
{
	zs_request_t *req;

	if (ctx->free_reqs != NULL) {
		req = ctx->free_reqs;
		ctx->free_reqs = ctx->free_reqs->next;

		return req;
	}

	zs_err("there is no available\n");

	return NULL;
}

void
zs_cleanup(zs_context_t *ctx, zs_request_t *req)
{
	int n;
	int on = 0;
	setsockopt(req->sockfd, SOL_TCP, TCP_CORK, &on, sizeof(on));

	ctx->connection_num--;

	n = epoll_ctl(ctx->epfd, EPOLL_CTL_DEL, req->sockfd, NULL);
	if (n < 0) {
		zs_err("epoll ctl del failed.\n");
	}

	req->next = ctx->free_reqs;
	ctx->free_reqs = req;
	//zs_destroy_pool(req->pool);
	zs_reset_pool(req->pool);

	//zs_err("[ Ed ] Process:%d sockfd:%d status:%d num:%d \n",
	//	ctx->process_i, req->sockfd, req->status, ctx->connection_num);

	close(req->sockfd);
	close(req->file_fd);
}

void 
zs_send_header(zs_context_t *ctx, int v, zs_request_t *req)
{
	int n, len;

	req->res_header = zs_palloc(req->pool, 256);
	lua_getglobal(ctx->Hdr, "get_header");
	lua_pushnumber(ctx->Hdr, req->res_code);
	lua_pushstring(ctx->Hdr, req->res_lastmod_f);

	n = lua_pcall(ctx->Hdr, 2, 1, 0);
	if (n != LUA_OK) {
		zs_err("get header error\n");
	}

	strcpy(req->res_header, lua_tostring(ctx->Hdr, -1));

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

    if (req->res_code != 304) {
		ctx->ee.events = EPOLLOUT | EPOLLET;
		ctx->ee.data.ptr = req;
		n = epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, req->sockfd, &ctx->ee);

		if (n < 0) {
			zs_err("send header mod epoll error.\n");
		}

	} else {
		zs_cleanup(ctx, req);
	}
}

void 
zs_send_cache_response(zs_context_t *ctx, zs_request_t *req) 
{
	int len, n;

	len = req->res_length;

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

	zs_cleanup(ctx, req);
}

void 
zs_get_request_line(zs_request_t *req)
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
	if (req->suffix == NULL) {
		req->suffix = "/";
	}

	/*
	 * get the if modified since time from request head.
	 */
	req->if_modified_since = zs_palloc(req->pool, 30);
	req->if_modified_since = strstr(req->buf, "If-Modified-Since: ");
	if (req->if_modified_since != NULL) {
		i = strcspn(req->if_modified_since, rn);
		req->if_modified_since[i] = '\0';
		i = strlen(req->if_modified_since) - 19;
		memmove(req->if_modified_since, req->if_modified_since + 19, i);
		req->if_modified_since[i] = '\0';	
		//zs_err("yang %s\n", req->if_modified_since);
	} 
}

int_t
zs_add_index_file(zs_context_t *ctx, zs_request_t *req) 
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

/*
void
zs_send_static_file(zs_request_t *req)
{
    int n;

    zs_send_header(1, req);

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
        zs_err("res fcnt is null\n"); 
    }

    zs_cleanup(ctx, req);
}

void
zs_process_static_file(zs_context_t *ctx, zs_request_t *req)
{
	int n;

    if (zs_is_in_cache(ctx, req) != ZS_OK) {
	    req->fp = fopen(req->pf, "rb");
	    if (req->fp != NULL) {
	        zs_read_static_file(ctx, req);     

	    } else {
	        zs_err("file not found");
	    }

	    return ;
	}

  	req->res_code = 200;
	req->status = ZS_WR_HEADER;
	req->in_cache = 1;

	zs_get_cache(ctx, req);
	req->in_cache = 1;

    ctx->ee.events = EPOLLOUT | EPOLLET;
    ctx->ee.data.fd = req->sockfd;
    req->status = ZS_SEND_STATIC;
    n = epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, req->sockfd, &ctx->ee);

    if (n < 0) {
        zs_err("fread epoll ctl error\n");  
    }
}
*/

int_t 
zs_is_same_modtime(zs_request_t *req) 
{
	int i, len;
	struct tm *timeinfo;

	req->res_lastmod_f = zs_palloc(req->pool, 80);
	timeinfo = gmtime(&req->res_lastmod);
	strftime(req->res_lastmod_f, 80, "%a, %d %b %G %T GMT", timeinfo);

	if (req->if_modified_since == NULL) {
		return ZS_NO;
	}

	len = strlen(req->if_modified_since);
	for (i = 0; i < len; i++) {
		if (req->if_modified_since[i] != req->res_lastmod_f[i]) {
			return ZS_NO;
		}
	}

	return ZS_OK;
}

int_t 
zs_run_get_method(zs_context_t *ctx, zs_request_t *req)
{
	int n, len;
    struct stat f;

	req->pf = zs_palloc(req->pool, 512);
	
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
		if (zs_init_apache(ctx, req) != ZS_OK) {
			return ZS_ERR; 
		}

	} else {
		if (zs_is_plain(req->suffix) == ZS_OK) {
			req->is_static_file = 0;

		} else if (zs_is_static_file(req->suffix) == ZS_OK) {
			req->is_static_file = 1;
		}

		/*
		 * to know the request content is in cahce 
		 */
		if (ctx->conf->use_cache == 1) {
			n = zs_is_in_cache(ctx, req);
			if (n == ZS_OK) {
				req->res_code = 200;   

				/* 
				 * read event end, start write event 
				 */
				req->status = ZS_WR_HEADER;
				req->in_cache = 1;

				/*
				 * if the cache file is modified, it nctx->eeds to update
				 * it before get cache.
				 */

				req->file_fd = open(req->pf, O_RDONLY);
				if (req->file_fd < 0) {
					zs_err("open error\n");
					return ZS_ERR;
				}

				fstat(req->file_fd, &f);
				req->modified_time = f.st_mtime;

				n = zs_is_cache_modified(ctx, req);
				if (n == ZS_OK) {
					zs_update_cache(ctx, req);
					zs_get_cache(ctx, req);

				}

				/*
				 * directly go to the end label.
				 */
				goto end;

			} else if (n == ZS_ERR) {
				return ZS_ERR;
			}
		}

		/*
		 * if not in cache, then do it normally.
		 */
		if (lstat(req->pf, &f) == -1) {
			req->res_code = 404;
			strcpy(req->pf, ctx->conf->page_404);
		}

		if (S_ISDIR(f.st_mode) && strlen(req->pf) != 1) {
			req->res_code = 404;
			strcpy(req->pf, ctx->conf->page_404);			
		}

		req->file_fd = open(req->pf, O_RDONLY);
		if (req->file_fd == -1) {
			req->res_code = 404; 

		} else {
			req->res_cnt = zs_palloc(req->pool, (1 << 20));
			fstat(req->file_fd, &f);
			req->modified_time = f.st_mtime;
			req->res_lastmod = f.st_mtime;

			if (zs_is_same_modtime(req) == ZS_OK) {
				/*
				 *  http code: 304   Not modified
 				 */
 				req->res_code = 304;

			} else {
				req->res_code = 200;  
			}

			if (ctx->conf->use_cache == 1){
				zs_store_cache(ctx, req);
			}

			/*
			 * read event end, start write event
			 */
			req->status = ZS_WR_HEADER;
		}

end:
		ctx->ee.events = EPOLLOUT | EPOLLET;
		ctx->ee.data.ptr = req;
		n = epoll_ctl(ctx->epfd, EPOLL_CTL_MOD, req->sockfd, &ctx->ee);

		if (n == -1) {
			zs_err("epoll modify error."); 
		}

		shutdown(req->sockfd, SHUT_RD);

	}

	return ZS_OK;
}

void 
zs_parse_start(zs_context_t *ctx, zs_request_t *req)
{
	int n;

	/*
	 * if the request mehtod is GET
	 */
    n = zs_run_get_method(ctx, req); 
    
    if (n == ZS_NO) {
        return; 
    }
}

void 
zs_parse_request(zs_context_t *ctx, zs_request_t *req)
{
	zs_get_request_line(req);

	zs_parse_start(ctx, req);	
}

void
zs_read_request(zs_context_t *ctx, zs_request_t *req)
{
	int_t n;

	req->buf = zs_palloc(req->pool, 1024);

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
	req->buf[req->has_read] = '\0';

	//zs_err("======\nHEADER: %s\n======\n", req->buf);
	
	/*
	 * start to parse the request
	 */
	zs_parse_request(ctx, req);

}

void
zs_send_response(zs_context_t *ctx, zs_request_t *req)
{
	int_t n;
	off_t offset;
	struct stat buf;

	fstat(req->file_fd, &buf);
	req->res_length = buf.st_size;  

	req->has_written = 0;
	while (1) {
		offset = req->has_written;
		n = sendfile(req->sockfd, req->file_fd, &offset, req->res_length - req->has_written); 
		req->has_written = offset; 

		if (n < 0) {
			if (errno == EAGAIN) {
				break; 

			} else {
				return ; 
			}
		} 

		if (req->has_written == req->res_length) {
			zs_cleanup(ctx, req);
			return;
		}
	}  
}

void 
zs_handle_request(zs_context_t *ctx, zs_request_t *req)
{				
	int_t n, on = 1;
	sock_t connfd, listenfd;
	zs_request_t  *newreq;
	struct sockaddr_in cliaddr;
	socklen_t socklen;

	listenfd = ctx->listen_sock.sockfd;   

	if (listenfd == req->sockfd) {
		/*
		if (ctx->conf->workers > 1) {
			if (flock(ctx->fd, LOCK_EX) < 0) {
				if (errno == EAGAIN || errno == EACCES) {

					
					 // if the file lock is held by another process, 
					 // then this porcess do nothing
					 
					zs_err("Process:%d has bctx->een locked!\n", ctx->process_i);
					return ;

				} else {
					zs_err("flock error\n");
					return ;
				}
			}
		} */

		/*
		 * the ctx->connection_num in this processes
		 * must less then the max num 
		 */
		if (ctx->connection_num < ctx->conf->worker_connections) {		
			while (1) {		

				/* 
				 * if the workers great than one , 
				 * then use file lock for only one process to 
				 * accept a new connection
				 */ 
				socklen = sizeof(cliaddr);
				connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &socklen);         

				if (connfd == -1) {
					if ((errno == EAGAIN) || (errno == EWOULDBLOCK))  {
						break; 

					} else if (errno == ECONNRESET){
						zs_err("Connction reset.\n");
						continue;

					} else {
						zs_err("accept error.\n");     
						break;
					}
				}
		
				ctx->connection_num++;

				n = zs_set_nonblocking(connfd);
				if  (n == -1) {
					return ; 
				}

				setsockopt(connfd, SOL_TCP, TCP_CORK, &on, sizeof(on));

				//zs_err("[ Ac ] Process:%d Num:%d connfd:%d\n", 
                //	ctx->process_i, ctx->connection_num, connfd);
				//ctx->reqs[sockfd] = zs_get_req(ctx, connfd);
				newreq = zs_get_req(ctx, connfd);
				if (newreq == NULL) {
					zs_err("null req\n");
					return ;
				}

				newreq->pool = zs_create_pool(1024);
				newreq->sockfd = connfd;
				newreq->status = ZS_RD_REQ;
				newreq->has_read = 0;
				newreq->has_written = 0;
				newreq->in_cache = 0;

				ctx->ee.data.ptr = newreq;
				ctx->ee.events = EPOLLIN | EPOLLET;
				n = epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, connfd, &ctx->ee);

				if (n == -1) {
					zs_err("connfd epoll add failed.\n"); 
					return;
				}

			}	
		}
		/*
		if (ctx->conf->workers > 1) {
			flock(ctx->fd, LOCK_UN);
		}*/

	} else {	

		switch (req->status) {

		case ZS_RD_REQ:  	
			zs_read_request(ctx, req);
			break;

		case ZS_WR_HEADER:
			zs_send_header(ctx, 0, req);    
			break;

		case ZS_WR_REQ:
			if (req->in_cache != 1) {
				zs_send_response(ctx, req);

			} else {
				zs_send_cache_response(ctx, req);
			}

			break;

		case ZS_RD_PHP:
			zs_read_php(ctx, req);
			break;

		case ZS_WR_PHP:
			zs_send_php(ctx,req);
			break;

		case ZS_WR_REQ_PHP:
			zs_write_req_to_php(ctx, req);
			break;
			/*
        case ZS_SEND_STATIC:
        	if (req->in_cache != 1) {
            	zs_send_static_file(req);

        	} else  {
        		zs_send_cache_response(req);
        	}
            break;*/
		}
	}
}