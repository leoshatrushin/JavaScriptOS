#include "syscalls.h"
#include "utils.h"

#define X(name) extern int sys_##name(void);
SYSCALL_LIST
#undef X

#define X(name) [SYS_##name] = sys_##name,
static int (*syscalls[])(void) = {
    SYSCALL_LIST
};
#undef X

trapframe *syscall_dispatch(trapframe *tf) {
    u64 syscall_num = tf->rsi;

    if(syscall_num < NELEM(syscalls) && syscalls[syscall_num]) {
        tf->rax = syscalls[syscall_num]();
    } else {
        tf->rax = -1;
    }

    return tf;
}
