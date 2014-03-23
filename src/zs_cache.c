
#include <zs_core.h>

/*
static void 
zs_read_static_file(zs_context_t *ctx, zs_request_t *req)
{
    int n;
	
    fseek(req->fp, 0, SEEK_END);
    req->res_length = ftell(req->fp);
    rewind(req->fp);
    zs_err("static file length: %d\n", req->res_length);

    req->has_read = 0;
    req->res_cnt = zs_palloc(req->pool, req->res_length);

    n = fread(req->res_cnt, 4096, req->res_length, req->fp);
    zs_err("fread chunks:%d\n", n);
}
*/

static void
zs_read_res_cnt(zs_context_t *ctx, zs_request_t *req)
{
	int n;
	struct stat buf;

	fstat(req->file_fd, &buf);
	req->res_length = buf.st_size;  
	req->res_cnt = zs_palloc(req->pool, (1 << 20));

	n = read(req->file_fd, req->res_cnt, req->res_length);

	if (n < 0) {
		zs_err("read file content for cache error.\n");
		return ;
	} 
}

int_t 
zs_is_in_cache(zs_context_t *ctx, zs_request_t *req)
{
	int n;

	lua_getglobal(ctx->L, "is_in_cache");
	lua_pushstring(ctx->L, req->pf);
	n = lua_pcall(ctx->L, 1, 1, 0);

	if (n != LUA_OK) {
		zs_err("is in cache lua pcall error.\n");
		return ZS_ERR;
	}

	return lua_tonumber(ctx->L, -1);
}

int_t 
zs_is_cache_modified(zs_context_t *ctx, zs_request_t *req)
{
	int n;

	lua_getglobal(ctx->L, "is_cache_modified");
	lua_pushstring(ctx->L, req->pf);
	lua_pushnumber(ctx->L, req->modified_time);
	n = lua_pcall(ctx->L, 2, 1, 0);

	if (n != LUA_OK) {
		zs_err("is cache modified error\n");
		return ZS_ERR;
	}

	if (lua_tonumber(ctx->L, -1) == 0) {
		return ZS_NO;
	} 

	return ZS_OK;
}

void
zs_update_cache(zs_context_t *ctx, zs_request_t *req)
{
	int n;

	/*
	 * read the file content.
	 */
	zs_read_res_cnt(ctx, req);

	lua_getglobal(ctx->L, "sync_content");
	lua_pushstring(ctx->L, req->pf);
	lua_pushstring(ctx->L, req->res_cnt);
	lua_pushnumber(ctx->L, req->modified_time);

	n = lua_pcall(ctx->L, 3, 0, 0);
	if (n != LUA_OK) {
		zs_err("update cache error\n");
		return ;
	}
}

int_t
zs_get_cache(zs_context_t *ctx, zs_request_t *req)
{
	int n, i;

	lua_getglobal(ctx->L, "get_cache");	
	lua_pushstring(ctx->L, req->pf);
	n = lua_pcall(ctx->L, 1, 2, 0);

	if (n == LUA_OK) {

		/*
		 * the page is not in cache
		 */
		req->cache_buf = zs_palloc(req->pool, (1 << 19));
		req->res_length = lua_tonumber(ctx->L, -1);

		/*
		 * because the content of static file will has '\0', 
		 * must use the length 
		 */
		req->has_read = 0;
		for (i = 0; i < req->res_length; i++) {
			req->cache_buf[i] = lua_tostring(ctx->L, -2)[i];
		}

	} else {
		zs_err("%s\n", lua_tostring(ctx->L, -1));
		return ZS_ERR;
	}

	return ZS_OK;
}

int_t
zs_store_cache(zs_context_t *ctx, zs_request_t *req)
{
	/* 
	 * if it is a static file 
	 */
	if (req->is_static_file == 0) {
		zs_read_res_cnt(ctx, req);

		lua_getglobal(ctx->L, "store_cache");
		lua_pushstring(ctx->L, req->pf);
		lua_pushstring(ctx->L, req->res_cnt);
		lua_pushnumber(ctx->L, req->modified_time);

		if (lua_pcall(ctx->L, 3, 0, 0) != 0) {
			zs_err("store cache lua_pcall error.\n");
			return ZS_ERR;
		}

	} else {
		lua_getglobal(ctx->L, "store_b_cache");
		lua_pushstring(ctx->L, req->pf);
		lua_pushnumber(ctx->L, req->modified_time);

		if (lua_pcall(ctx->L, 2, 0, 0) != 0) {
			zs_err("store b cache error\n");
			return ZS_ERR;
		}
	}

	return ZS_OK;
}
