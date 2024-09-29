#pragma once
#include "idt.h"
#include <kernel/libk/types.h>
#include <kernel/arch/x86/paging.h>

typedef struct {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
} cpuid_result;

static inline cpuid_result cpuid_raw(u32 leaf, u32 subleaf = 0) {
    cpuid_result result;
    asm volatile("cpuid"
                 : "=a" (result.eax), "=b" (result.ebx), "=c" (result.ecx), "=d" (result.edx)
                 : "a" (leaf), "c" (subleaf)
                 : "memory");
    return result;
}

static inline void lcr3(cr3 val) {
    asm volatile("movq %0,%%cr3" : : "r" (val));
}

static inline void lidt(idtdesc idt[], u16 size) {
    IDTR idtr = {
        .lenm1 = (u16)(size - 1),
        .offset = (uintptr_t)idt
    };
  
    asm volatile("lidt (%0)" : : "r" (idtr));
}

static inline u64 rdmsr(u32 msr) {
    u32 lo, hi;
    asm volatile("rdmsr" : "=a" (lo), "=d" (hi) : "c" (msr));

    return (u64)hi << 32 | lo;
}

static inline void wrmsr(u32 msr, u64 val) {
    u32 lo = (u32)val;
    u32 hi = (u32)(val >> 32);
    asm volatile("wrmsr" : : "c" (msr), "a" (lo), "d" (hi));
}

static inline void ltr(u16 sel) {
    asm volatile("ltr %0" : : "r" (sel));
}

static inline void out8(u16 port, u8 val) {
    asm volatile("outb %0,%1" : : "a" (val), "Nd" (port));
}

static inline void out16(u16 port, u16 val) {
    asm volatile("outw %0,%1" : : "a" (val), "Nd" (port));
}

static inline void out32(u16 port, u32 val) {
    asm volatile("outl %0,%1" : : "a" (val), "Nd" (port));
}

static inline u8 in8(u16 port) {
    u8 val;
    // N constraint = x86 unsigned 8-bit integer constant for in and out instructions
    asm volatile("inb %1,%0" : "=a" (val) : "Nd" (port));
    return val;
}

static inline u16 in16(u16 port) {
    u16 val;
    asm volatile("inw %1,%0" : "=a" (val) : "Nd" (port));
    return val;
}

static inline u32 in32(u16 port) {
    u32 val;
    asm volatile("inl %1,%0" : "=a" (val) : "Nd" (port));
    return val;
}

static inline void hlt() {
    asm volatile("hlt");
}
