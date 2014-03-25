
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

void
zs_strncpy(char *dst, const char *src, int_t len) 
{
    int i;

    for (i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

void
zs_strcpy(char *dst, const char *src) 
{
    int i;

    for (i = 0; src != '\0'; i++) {
        dst[i] = src[i];
    }
}