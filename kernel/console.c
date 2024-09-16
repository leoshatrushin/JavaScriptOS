#include "asm.h"
#include "../../../libc/stdio.h"

// "debugcon"
void bochs_debug_output(char *s) {
    while(*s) {
        out8(0xe9, *s++);
    }
}

void debug_printf(const char *fmt, ...) {
    char buf[1024];
    va_list fmt_args;
    va_start(fmt_args, fmt);
    vsprintf(buf, fmt, fmt_args);
    va_end(fmt_args);
    bochs_debug_output(buf);
}
