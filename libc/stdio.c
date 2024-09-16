#include "stdio.h"

char *strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

static char digits[] = "0123456789abcdef";

static int sprintbase(char *buf, unsigned long number, unsigned char base) {
    char* bufleft = buf;
    int len = 0;
    if (number == 0) {
        *buf++ = '0';
        len++;
    }
    while (number > 0) {
        int digit = number % base;
        number /= base;
        *buf++ = digits[digit];
        len++;
    }
    *buf-- = '\0';
    // reverse the string
    while (bufleft < buf) {
        char tmp = *bufleft;
        *bufleft = *buf;
        *buf = tmp;
        bufleft++;
        buf--;
    }
    return len;
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    char* bufstart = buf;
    while (*fmt) {
        if (*fmt != '%') {
            *buf++ = *fmt++;
            continue;
        }
        fmt++;
        switch (*fmt++) {
            case 'u': {
                int number = va_arg(args, unsigned long);
                int len = sprintbase(buf, number, 10);
                buf += len;
                break;
            }
            case 'x': {
                int number = va_arg(args, unsigned long);
                *buf++ = '0';
                *buf++ = 'x';
                int len = sprintbase(buf, number, 16);
                buf += len;
                break;
            }
            case 's': {
                char *arg_string = va_arg(args, char*);
                char* end = strcpy(buf, arg_string);
                buf = end;
                break;
            }
        }
    }
    *buf = '\0';
    return buf - bufstart;
}

int sprintf(char *buf, const char *fmt, ...) {
    va_list fmt_args;
    va_start(fmt_args, fmt);
    int len = vsprintf(buf, fmt, fmt_args);
    va_end(fmt_args);
    return len;
}
