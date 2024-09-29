#include "../console.h"
#include "../arch/x86/asm.h"

void __assertion_failed(char const* msg, char const* file, unsigned line, char const* func)
{
    debug_printf("ASSERTION FAILED: {}\n", msg);
    debug_printf("{}:{} in {}\n", file, line, func);
    hlt();
}
