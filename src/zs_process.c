
#include <zs_core.h>

void 
zs_worker_process_init(zs_context_t *ctx)
{
	int i;
	//struct rlimit rl;

	//rl.rlim_cur = (rlim_t)((1 << 16) - 1);
	//rl.rlim_max = (rlim_t)((1 << 16) - 1);

	//if (setrlimit(RLIMIT_NOFILE, &rl) != 0) {
	//	zs_err("set resource limit error.\n"); 
	//}

	ctx->Hdr = luaL_newstate();
	luaL_openlibs(ctx->Hdr);
	if (luaL_dofile(ctx->Hdr, "./src/zs_header.lua") == -1) {
		zs_err("not found zs_header.lua");
		lua_close(ctx->Hdr);
		return ;
	}

	ctx->fd = open("./src/znuserv.c", O_WRONLY);
	ctx->L = luaL_newstate();
	luaL_openlibs(ctx->L);
	if (luaL_dofile(ctx->L, "./src/zs_cache.lua") == -1) {
		zs_err("not found zs_cache.lua");
		lua_close(ctx->L);
		return ;
	}

	lua_getglobal(ctx->L, "init");
	lua_pushnumber(ctx->L, ctx->conf->cache);
	if (lua_pcall(ctx->L, 1, 0, 0) != 0) {
		zs_err("%s\n", lua_tostring(ctx->L, 1));
		return ;
	}

	ctx->reqs = zs_palloc(ctx->pool, ctx->conf->worker_connections * sizeof(zs_request_t));
	if (ctx->reqs == NULL) {
		zs_err("No enough memory for reqs!\n");
		return ;
	}

	/* initial reqs array list */
	for (i = 0; i < ctx->conf->worker_connections - 1; i++) {
		ctx->reqs[i].next = &ctx->reqs[i + 1];
	}
	ctx->reqs[i].next = NULL;
	ctx->free_reqs = ctx->reqs;
}

static void
zs_worker_process_loop(zs_context_t *ctx, int i)
{
	zs_worker_process_init(ctx);

	for ( ; ; ) {
	   if (zs_process_event(ctx, i) < 0) {
		   zs_err("process event failed.\n");      
		   return;
	   }       

		/*worker processes receive signal...*/
	}
}

static void
zs_spawn_worker_process(zs_context_t *ctx, int i)
{
	//processes[i].pid = getpid();
	//zs_worker_process_loop(ctx, i);

	pid_t  pid;

	pid = fork();
	switch(pid) {
		
	case -1:
		zs_err("worker process fork error.\n");
		return ;

	case 0:
		zs_worker_process_loop(ctx, i);
		break;
	
	default: 
		ctx->cld_pid[i] = pid;
		break;
	}
}

void 
zs_worker_process(zs_context_t *ctx)
{
	int_t i, num_workers;
	
	num_workers = ctx->conf->workers;
	ctx->cld_pid = zs_palloc(ctx->pool, sizeof(int_t) * ctx->conf->workers);

	for (i = 0; i < num_workers; i++) {
		zs_spawn_worker_process(ctx, i);  
	}

	zs_write_pid(ctx);
}

void
zs_write_pid(zs_context_t *ctx)
{
	int_t fd, n, i;
	char p[31];

	fd = open(ctx->conf->pid, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		zs_err("open pid file error\n");
	}

	/*
	 * worker process pid
	 */
	for (i = 0; i < ctx->conf->workers; i++) {
		sprintf(p, "%d", ctx->cld_pid[i]);
		p[strlen(p)] = '\n';
		n = write(fd, p, strlen(p));
		if (n < 0) {
			zs_err("write worker process pid error\n");
		}
	}

	/*
	 * master process pid
	 */
	sprintf(p, "%d", getpid());
	p[strlen(p)] = '\n';
	n = write(fd, p, strlen(p));
	if (n < 0) {
		zs_err("write master process pid error\n");
	}

	close(fd);
}

void 
zs_master_process(zs_context_t *ctx)
{
	sigset_t sigset;

	/*
	 * Blocking some signals.
	 */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGHUP);
	//sigaddset(&sigset, SIGINT);
	//sigaddset(&sigset, SIGIO);
	sigaddset(&sigset, SIGALRM);

	if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1) {
		zs_err("sigprocmask error.\n"); 
	}

	if (zs_listen(ctx) == ZS_ERR) {
		zs_err("master process listen error.\n"); 
	}

	/* start worker process */
	zs_worker_process(ctx);

	/*
	 * Master process suspend....
	 */
	sigemptyset(&sigset);
	for (;;) {
		sigsuspend(&sigset);
	}
}

