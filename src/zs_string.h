
#ifndef _ZS_STRING_H
#define _ZS_STRING_H

#include <zs_core.h>

void zs_strncpy(char *dst, const char *src, int_t len);
void zs_strcpy(char *dst, const char *src);
void zs_vfprintf(const char *fmt, ...);
int_t zs_err(const char *fmt, ...);


#endif
