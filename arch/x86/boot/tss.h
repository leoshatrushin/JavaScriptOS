#pragma once
#include "types.h"

// On x86, the TSS was for hardware task switching, but this proved slower than software task switching
// On x86-64 the TSS is used for interrupt stack switching instead, to prevent kernel data leaks
typedef struct {
    u32 rsrvz0;
    u64 rsp0; // if idtdesc->cs.dpl = X is more privileged, switch to stack rspX before pushing iret frame
    u64 rsp1;
    u64 rsp2;
    u64 rsrvz1;
    u64 rsrvz2;
    u64 ist1; // Interrupt Stack Table
    u64 ist2; // if idtdesc->ist = X > 0, switch to stack istX before pushing iret frame, overrides rspX
    u64 ist3; // useful for some interrupts that can occur at any time, like a machine check or NMI
    u64 ist4; // or if you do sensitive work in a specific interrupt and don't want to leak data
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 rsrvz3;
    u16 rsrvz4;
    u16 io_bitmap_offset; // if EFLAGS.IOPL < CPL, IO port access is not allowed
                          // otherwise can allow by setting a bit in a bitmap
                          // put right after TSS?
} __attribute__((__packed__)) tssseg_t;

// TSS and SMP
// there is nothing stopping two cores from both starting executing at rsp0
// options:
// - separate GDT per code
// - single GDT, separate TSS
// - single GDT and single TSS, TR caching
