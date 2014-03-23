
#include <zs_core.h>


extern int_t zs_deamon();


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

