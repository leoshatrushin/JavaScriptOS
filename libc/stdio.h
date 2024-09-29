#pragma once
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

char *strcpy(char *dst, const char *src);
int vsprintf(char *buffer, const char *fmt, va_list args);
int sprintf(char *buffer, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
