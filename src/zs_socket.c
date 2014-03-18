
#include <zs_core.h>

int_t
zs_set_nonblocking(sock_t s)
{
    int_t val; 

    if ((val = fcntl(s, F_GETFL, 0)) < 0) {
        zs_err("fcntl F_GETFL error.\n");      
    }

    return fcntl(s, F_SETFL, val | O_NONBLOCK);
}

int_t 
zs_listen(zs_context_t *ctx)
{
    /*
    struct addrinfo hints, *res, *r;
    sock_t listenfd;
    int_t n, on = 1;
    const char *host;

    host = ctx->conf->server_name;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((n = getaddrinfo(host, NULL, &hints, &res)) != 0) {
        zs_err("getaddrinfo error.\n"); 
    }

    r = res;

    do {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); 
        if (listenfd < 0) {
            zs_err("listen fd is less then 0.\n"); 
        }

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) {
            zs_err("%s\n", res->ai_addr->sa_data);
            break; 
        }

        close(listenfd);
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        zs_err("listen error"); 
    }

    listen(listenfd, LISTENQ);

    freeaddrinfo(r);
    */

    int_t on = 1, n;
    sock_t listenfd;    
    struct sockaddr_in servaddr;
    struct hostent *hptr;

    //zs_err("server name is %s\n", ctx->conf->server_name);
    if ((hptr = gethostbyname(ctx->conf->server_name)) == NULL){
        zs_err("gethostbyname error.\n");
        return ZS_ERR;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ctx->conf->listen_port);
    servaddr.sin_addr = *(struct in_addr *)hptr->h_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));

    /* set the listenfd to nonblocking. */
    if (zs_set_nonblocking(listenfd) < 0) {
        if (close(listenfd) < 0) {
           zs_err("close listenfd error.\n");  
        }

        zs_err("set nonblocking error.\n"); 
    }

    n = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if ( n != 0) {
        zs_err("bind error.\n"); 

        if (close(listenfd) < 0) {
            zs_err("bind error and close listenfd error.\n"); 
        }

        return ZS_ERR;
    }

    if (listen(listenfd, LISTENQ) != 0) {
        zs_err("listen error.\n"); 

        if (close(listenfd) < 0) {
            zs_err("bind error and close listenfd error.\n"); 
        }

        return ZS_ERR;
    }

    ctx->listen_sock.sockfd = listenfd;
    ctx->listen_sock.local_sa.sin = servaddr;
    
    return ZS_OK;
}

