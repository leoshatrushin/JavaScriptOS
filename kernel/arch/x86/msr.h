#pragma once

#ifndef __ASSEMBLER__
#include "types.h"

typedef struct {
    u8  rsrv0;
    u8  bsp : 1; // processor is bootstrap processor
    u8  rsrv1 : 1;
    u8  x2apic_enable : 1;
    u8  xapic_enable : 1;
    u32 mmio_base : 24;
    u32 rsrv2;
} msr_ia32_apic_base_t;

#endif // __ASSEMBLER__

#define MSR_IA32_APIC_BASE 0x1B

#define MSR_EFER 0xC0000080 // Extended Feature Enable Register
#define MSR_EFER_LONG_MODE (1 << 8) // Long mode enable
