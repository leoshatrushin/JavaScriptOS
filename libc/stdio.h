#pragma once
#include <stdarg.h>

char *strcpy(char *dst, const char *src);
int vsprintf(char *buffer, const char *fmt, va_list args);
int sprintf(char *buffer, const char *fmt, ...);
