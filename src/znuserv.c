
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

    /*
    zs_timer_t *timer, t;
    t.pool = zs_create_pool(4096);

    if (t.pool == NULL) {
        zs_err("timer pool create failed.\n"); 
        return 0;
    }

    timer = &t;

    if (zs_timer_init(timer) != ZS_OK) {
        return 0; 
    }

    time_t ta, tb, tc;
    struct tm tmb, tmc;

    time(&ta);
    tmb = *localtime(&ta);
    tmc = tmb;
    
    tmb.tm_min++;
    tmc.tm_hour++;
    tb = mktime(&tmb);
    tc = mktime(&tmc);

    zs_timer_add_node(timer, ta);
    zs_timer_add_node(timer, tb);
    zs_timer_add_node(timer, tc);

    zs_err("current time is : %s", ctime(&timer->node[0].time));
    zs_err("current time is : %s", ctime(&timer->node[1].time));
    zs_err("current time is : %s", ctime(&timer->node[2].time));

    int i;
    for (i = 0; i < timer->length; i++) {
        zs_err("%s", ctime(&timer->node[i].time)); 
    }

    zs_timer_del_min(timer);
    zs_err("\n");

    for (i = 0; i < timer->length; i++) {
        zs_err("%s", ctime(&timer->node[i].time)); 
    }
    
    time_t tt;
    time(&tt);
    zs_timer_add_node(timer, tt);
    struct tm new;

    new = *localtime(&tt);

    new.tm_min += 1;

    zs_err("%f", difftime(tt, mktime(&new)));
    */


    /*
     * 在Linux下写socket的程序的时候，
     * 如果尝试send到一个disconnected socket上，
     * 就会让底层抛出一个SIGPIPE信号.
     *
     * 这个信号的缺省处理方法是退出进程, 这都不是我们期望的。
     * 因此我们需要重载这个信号的处理方法。
     */
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

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

