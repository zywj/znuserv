
#include <zs_core.h>


volatile zs_timer_t *timer;


void zs_heapsrot();
static void zs_minheap(int_t i, int_t len);
static void zs_build_minheap();

int_t
zs_timer_init() 
{    
    timer->node = zs_palloc(timer->pool, sizeof(struct zs_timer_node_s));
    
    if (timer->node == NULL) {
        zs_err("timer node alloc failed.\n"); 
        return ZS_ERR;
    }

    timer->node[0].time = 0;
    timer->length = 0; 

    return ZS_OK;
}

void 
zs_heapsort()
{
    zs_build_minheap();

    int_t i, len;
    time_t tmp;

    len = timer->length;

    for (i = timer->length; i > 1; i--) {
        tmp = timer->node[i - 1].time; 
        timer->node[i - 1].time = timer->node[0].time;
        timer->node[0].time = tmp;
        len--;

        zs_minheap(1, len);
    }
}

int_t
zs_timer_add_node(time_t time)
{
    timer->pool = zs_palloc(timer->pool, sizeof(struct zs_timer_node_s));
    /*
    if (timer->length >= 1024) {
        zs_err("too many timer event.\n"); 
        return ZS_ERR;
    }
    */

    timer->node[timer->length].time = time;
    timer->length++;

    zs_heapsort();

    return ZS_OK;
}

static void
zs_minheap(int_t i, int_t len)
{
    int_t l, r, largest;
    time_t tmp;

    l = LEFT(i);
    r = RIGHT(i);
    
    if (l <= len && timer->node[l - 1].time < timer->node[i - 1].time) {
        largest = l; 
    } else {
        largest = i; 
    }

    if (r <= len && timer->node[r - 1].time < timer->node[largest - 1].time) {
        largest = r; 
    }

    if (largest != i) {
        tmp = timer->node[i - 1].time;
        timer->node[i - 1].time = timer->node[largest - 1].time;
        timer->node[largest - 1].time = tmp;

        zs_minheap(largest, len);
    }
}

static void 
zs_build_minheap()
{
    int_t i;
    
    for (i = timer->length / 2; i > 0; i--) {
        zs_minheap(i, timer->length); 
    }
}


int_t 
zs_timer_del_node(int m)
{
    int_t i = m;                                     
    int_t j = 2 * i + 1;
    int_t flag;
    time_t tmp = timer->node[i].time;

    flag = j;
    while (j < timer->length) {

        if (j + 1 < timer->length && timer->node[j].time > timer->node[j + 1].time) 
            j++;

        if (timer->node[j].time >= tmp)
            break;

        else  {
            timer->node[i].time = timer->node[j].time;
            i = j;
            j = 2 * i + 1;
        }
    }

    if (flag == j)
        return ZS_ERR;

    return ZS_OK;
}

inline struct zs_timer_node_s 
zs_timer_del_min()
{
    struct zs_timer_node_s d;

    timer->length--;
    d = timer->node[timer->length];
    zs_heapsort();

    return d;
}

inline struct zs_timer_node_s 
zs_timer_get_min()
{
    return timer->node[timer->length - 1];
}

