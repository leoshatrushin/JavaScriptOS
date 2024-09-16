#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

char *strcpy(char *dst, const char *src) {
    size_t i;
    for (i = 0; src[i]; ++i) {
        dst[i] = src[i];
    }
    dst[i] = 0;
    return dst;
}

size_t strncpy(char *dst, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; ++i) {
        dst[i] = src[i];
    }
    if (i < n) {
        dst[i] = 0;
    }
    return i;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) {
            return (char*)s;
        }
        s++;
    }
    return 0;
}
