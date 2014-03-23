
#include <zs_core.h>

/*
 * default value for configure arguments
 */
#define DF_LISTEN_PORT 8080
#define DF_SERVER_NAME "localhost"
#define DF_INDEX_FILES "index.htm"
#define DF_ROOT_DIR "./html/"
#define DF_WORKERS 1
#define DF_WORKER_CONNECTIONS 1024
#define DF_EVENT_TIMEOUT 400
#define DF_PHP_LISTEN_PORT 81
#define DF_CACHE  100
#define DF_IS_DEAMON 1
#define DF_USE_CACHE 1
#define DF_PAGE_404 "./html/404.html"

#define ZS_MAX_PROCESSES 1024
#define ZS_MAX_UNSIGNED 65535


enum {LISTEN_PORT, SERVER_NAME, INDEX_FILES, ROOT_DIR, WORKERS, WORKER_CONNETIONS, EVENT_TIMEOUT, PHP_LISTEN_PORT,
       CACHE, IS_DEAMON, USE_CACHE, PAGE_404};
const char *config_option[] = {
    "listen_port",
    "server_name",
    "index_files",
    "root_dir",
    "workers",
    "worker_connections",
    "event_timeout",
    "php_listen_port",
    "cache",
    "is_deamon",
    "use_cache",
    "page_404",
    "NULL"
};


int_t
zs_get_config(zs_context_t *ctx)
{
	int i, tmp, idx_file_err;
    char *config_file = "./conf/zs.config";

    lua_State *L;    
    L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, config_file)) {
        zs_err("Couldn't load file: %s\n", lua_tostring(L, -1)); 
        lua_close(L);
        return ZS_NO;
    }

    i = 0;
    while (config_option[i][0] != 'N' &&
            config_option[i][1] != 'U' &&
            config_option[i][2] != 'L' &&
            config_option[i][3] != 'L') {
        switch(i) {

        case LISTEN_PORT:
            lua_getglobal(L, "listen_port");

            if ((tmp = lua_tonumber(L, -1)) <= 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_LISTEN_PORT;
                zs_err("ERROR. The argument *listen port* is error. "
                        "It has been set default value.\n");
            }

            ctx->conf->listen_port = tmp;
            break;

        case PHP_LISTEN_PORT:
            lua_getglobal(L, "php_listen_port");

            if ((tmp = lua_tonumber(L, -1)) <= 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_PHP_LISTEN_PORT;
                zs_err("ERROR. The argument *php listen port* is error. "
                        "It has been set default value.\n");
            }

            ctx->conf->php_listen_port = tmp;
            break;

        case CACHE:
            lua_getglobal(L, "cache");

            if ((tmp = lua_tonumber(L, -1)) <= 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_CACHE;
                zs_err("ERROR. The argument *cache* is error. "
                        "It has been set default value.\n");
            }

            ctx->conf->cache = tmp;
            break;

        case SERVER_NAME:
            lua_getglobal(L, "server_name");

            ctx->conf->server_name = zs_palloc(ctx->pool, 127);
            if (strcpy(ctx->conf->server_name, lua_tostring(L, -1)) == NULL || ctx->conf->server_name[0] == '\0') {
                strcpy(ctx->conf->server_name, DF_SERVER_NAME); 
                zs_err("ERROR. The argument *server name* is error. "
                        "It has been set default value.\n");
            } 

            break;

        case INDEX_FILES:
            lua_getglobal(L, "index_files");
            idx_file_err = 0;

            if (lua_istable(L, -1) == 0) {
                idx_file_err = 1;
                ctx->conf->index_files[0] = DF_INDEX_FILES;
                zs_err("Something wrong with the index file argument."
                        "It has been set default value.\n");                 
            }

            if (idx_file_err == 0) {
                int k = 0;
                lua_pushnil(L);

                while (lua_next(L, -2) != 0) {
                    char *tt;
                    tt = zs_palloc(ctx->pool, 127);
                    strcpy(tt, lua_tostring(L, -1)); 
                    lua_pop(L, 1);
                    ctx->conf->index_files[k++] = tt;
                }
            }

            break;

        case ROOT_DIR:
            lua_getglobal(L, "root_dir");

            ctx->conf->root_dir = zs_palloc(ctx->pool, 127);
            if (strcpy(ctx->conf->root_dir, lua_tostring(L, -1)) == NULL || ctx->conf->server_name[0] == '\0') {
                strcpy(ctx->conf->root_dir, DF_ROOT_DIR);
                zs_err("ERROR. The argument *root dir* is error. "
                        "It has been set default value.\n");
            } 

            break;

        case WORKERS:
            lua_getglobal(L, "workers");

            if ((tmp = lua_tonumber(L, -1)) < 0 || tmp > ZS_MAX_PROCESSES) {
                tmp = DF_WORKERS; 
                zs_err("ERROR. The argument *workers* is error. "
                        "It has been set default value.\n");
            } 

            ctx->conf->workers =  tmp;
            break;

        case WORKER_CONNETIONS:
            lua_getglobal(L, "worker_connections");

            if ((tmp = lua_tonumber(L, -1)) < 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_WORKER_CONNECTIONS; 
                zs_err("ERROR. The argument *worker connections* is error. "
                        "It has been set default value.\n");
            } 

            ctx->conf->worker_connections =  tmp;
            break;

        case EVENT_TIMEOUT:
            lua_getglobal(L, "event_timeout");

            if ((tmp = lua_tonumber(L, -1)) < 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_EVENT_TIMEOUT; 
                zs_err("ERROR. The argument *event timeout* is error. "
                        "It has been set default value.\n");
            } 

            ctx->conf->event_timeout =  tmp;
            break;

        case IS_DEAMON:
            lua_getglobal(L, "is_deamon");

            if ((tmp = lua_tonumber(L, -1)) < 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_IS_DEAMON; 
                zs_err("ERROR. The argument *is deamon* is error. "
                        "It has been set default value.\n");
            } 

            ctx->conf->is_deamon =  tmp;
            break;

        case USE_CACHE:
            lua_getglobal(L, "use_cache");

            if ((tmp = lua_tonumber(L, -1)) < 0 || tmp > ZS_MAX_UNSIGNED) {
                tmp = DF_USE_CACHE; 
                zs_err("ERROR. The argument *use cache* is error. "
                        "It has been set default value.\n");
            } 

            ctx->conf->use_cache =  tmp;
            break;

        case PAGE_404:
            lua_getglobal(L, "page_404");

            ctx->conf->page_404 = zs_palloc(ctx->pool, 127);
            if (strcpy(ctx->conf->page_404, lua_tostring(L, -1)) == NULL || ctx->conf->server_name[0] == '\0') {
                strcpy(ctx->conf->page_404, DF_PAGE_404);
                zs_err("ERROR. The argument *page 404* is error. "
                        "It has been set default value.\n");
            } 

            break;
        }

        i++;
    }

    lua_close(L);
    return ZS_OK;
}

