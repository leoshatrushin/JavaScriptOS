#pragma once
#include "idt.h"

// syscalls can be done in several ways
// - dedicated DPL3 interrupt vectors (most common)
// - multiple dedicated vectors (but leaves less space for devices, a single PCI device can request up to 32)
// - dedicated niche instructions (e.g. sysenter), faster but more security-prone
// - execute a bad instruction to trigger an exception
// in microkernels, syscalls can be more complicated, as they might be redirected to other supervisor
// or even user programs

// System Call ABI
// - how to communicate syscall number
// - how to pass arguments
// - how to return values
// - how to pass large arguments

#define SYSCALL_LIST \
    X(fork) \
    X(exit) \
    X(wait) \
    X(pipe) \
    X(read) \
    X(kill) \
    X(exec) \
    X(fstat) \
    X(chdir) \
    X(dup) \
    X(getpid) \
    X(sbrk) \
    X(sleep) \
    X(uptime) \
    X(open) \
    X(write) \
    X(mknod) \
    X(unlink) \
    X(link) \
    X(mkdir) \
    X(close)

#define X(name) SYS_##name,
enum {
    SYSCALL_LIST
};
#undef X
