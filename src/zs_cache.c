
#include <zs_core.h>

int_t 
zs_is_in_cache(zs_context_t *ctx, zs_request_t *req)
{
	int n;

	lua_getglobal(ctx->L, "is_in_cache");
	lua_pushstring(ctx->L, req->pf);
	n = lua_pcall(ctx->L, 1, 1, 0);

	if (n == ZS_OK) {
		return ZS_OK;

	} 

	return ZS_NO;
}

int_t
zs_get_cache(zs_context_t *ctx, zs_request_t *req)
{
	int n;

	lua_getglobal(ctx->L, "get_cache");	
	lua_pushstring(ctx->L, req->pf);
	if (req->res_cnt != NULL) {
		lua_pushstring(ctx->L, req->res_cnt);

	} else if (req->res_fcnt != NULL) {
		lua_pushstring(ctx->L, req->res_fcnt);

	} 
	lua_pushnumber(ctx->L, req->modified_time);
	n = lua_pcall(ctx->L, 3, 1, 0);

	if (n == LUA_OK) {

		/*
		 * the page is not in cache
		 */
		if (lua_tonumber(ctx->L, -1) == ZS_ERR) {
			zs_err("no page in cache\n");
			return ZS_NO;
		}

		req->cache_buf = zs_palloc(req->pool, 1024);
		memcpy(req->cache_buf, lua_tostring(ctx->L, -1), strlen(lua_tostring(ctx->L, -1)) + 1);
		req->cache_buf[strlen(req->cache_buf)] = '\0';
		//zs_err("%s\n", req->cache_buf);

	} else {
		zs_err("%s\n", lua_tostring(ctx->L, -1));
		return ZS_ERR;
	}

	return ZS_OK;
}

static void
zs_read_res_cnt(zs_request_t *req)
{
	int n;

	n = read(req->file_fd, req->res_cnt, (1 << 20));

	if (n < 0) {	
		zs_err("read file content for cache error.\n");
		return ;
	} 
}

int_t
zs_store_cache(zs_context_t *ctx, zs_request_t *req)
{
	/* 
	 * read the html file content from req->file_fd
	 */
	zs_read_res_cnt(req);

	lua_getglobal(ctx->L, "store_cache");
	lua_pushstring(ctx->L, req->pf);
	lua_pushstring(ctx->L, req->res_cnt);
	lua_pushnumber(ctx->L, req->modified_time);

	if (lua_pcall(ctx->L, 3, 0, 0) != 0) {
		zs_err("store cache lua_pcall error.\n");
		return ZS_ERR;
	}

	return ZS_OK;
}