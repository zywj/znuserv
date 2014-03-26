
#include <zs_core.h>


extern int_t zs_deamon();

int_t
zs_stop(zs_context_t *ctx)
{
    int_t fd, pid, n, i, j, count;
    struct stat f;
    char c[128], t[128];

    if (lstat(ctx->conf->pid, &f) == -1) {
        return ZS_NO;
    }
    
    fd = open(ctx->conf->pid, O_RDONLY);
    n = read(fd, c, 128);
    if (n <= 0) {
        return ZS_NO;
    }

    i = 0;
    count = 0;
    while(1) {
        if (count == ctx->conf->workers + 1) {
            break;
        }

        for (j = 0; c[i] != '\n'; j++, i++) {
            t[j] = c[i];
        } 

        pid = atoi(t);
        n = kill(pid, SIGTERM);
        if (n < 0) {
            zs_err("error, when killing the pid signal\n");
            return ZS_NO;
        }

        count++;
        i++;
    }

    fd = open(ctx->conf->pid, O_CREAT | O_TRUNC);

    return ZS_NO;
}

int_t 
zs_get_option(zs_context_t *ctx, char **argv) 
{
    if (argv[1][0] == '-' && argv[1][1] == 's') {
        if (zs_stop(ctx) != ZS_OK) {
            return ZS_ERR;
        }

        return ZS_NO;

    } else {
        zs_err("The arguments is invalid.\n");
    }

    return ZS_NO;
}

int 
main(int argc, char *argv[])
{
    zs_context_t *ctx, ct;

    ct.pool = zs_create_pool(1024);
    ct.conf = zs_palloc(ct.pool, sizeof(zs_conf_t));
    ctx = &ct;
    
    if (zs_get_config(ctx) != ZS_OK) {
        return 0;
    }    

    if (argc > 1 && zs_get_option(ctx, argv) != ZS_OK) {
        return 0;
    }

    zs_context_init(ctx);

    if (ctx->conf->is_deamon == 1) {
        if (zs_deamon() != ZS_OK) {
            zs_err("Cann't be deamoned!\n"); 

        } else {
            zs_err("It has been deamoned.\n"); 
        }
    }

    zs_master_process(ctx);

    return 0;
}

