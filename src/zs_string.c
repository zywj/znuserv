
#include <zs_core.h>

void 
zs_vfprintf(const char *fmt, ...)
{
    FILE *fp;

    fp = stderr;

    va_list vl;
    va_start(vl, fmt);
    vfprintf(fp, fmt, vl);
    va_end(vl);
}

int_t 
zs_err(const char *fmt, ...)
{
    FILE *fp;

    fp = stderr;

    va_list vl;
    va_start(vl, fmt);
    vfprintf(fp, fmt, vl);
    va_end(vl);

    return ZS_ERR;
}
