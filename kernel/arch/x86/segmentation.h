#pragma once
#ifndef __ASSEMBLER__
#include "../../libk/types.h"

typedef enum {
    DPL_KERNEL = 0,
    DPL_USER = 3
} dpl;

// Segment Selector
typedef struct {
    u8  rpl : 2;        // Requested Privilege Level, useful in some edge cases but best set to target DPL
    u8  ti : 1;         // Table Indicator, 0 = GDT, 1 = LDT
    u32 index : 13;     // GDT index
} segsel;

#define SEGSEL_NULL     ((segsel){0, 0, 0}) // null segment
#define SEGSEL_KCODE    ((segsel){0, 0, 1}) // kernel code
#define SEGSEL_KDATA    ((segsel){0, 0, 2}) // kernel data+stack
#define SEGSEL_UCODE    ((segsel){0, 0, 3}) // user code
#define SEGSEL_UDATA    ((segsel){0, 0, 4}) // user data+stack
#define SEGSEL_TSS_LOW  ((segsel){0, 0, 5}) // TSS desc low 64 bits
#define SEGSEL_TSS_HIGH ((segsel){0, 0, 6}) // TSS desc high 64 bits

#define SEGIDX_NULL     0 // null segment
#define SEGIDX_KCODE    1 // kernel code
#define SEGIDX_KDATA    2 // kernel data+stack
#define SEGIDX_UCODE    3 // user code
#define SEGIDX_UDATA    4 // user data+stack
#define SEGIDX_TSS_LOW  5 // TSS desc low 64 bits
#define SEGIDX_TSS_HIGH 6 // TSS desc high 64 bits

enum sysdesc_type {
    SYSDESC_TYPE_LDT = 2,
    SYSDESC_TYPE_TSS_AVAIL = 9,
    SYSDESC_TYPE_TSS_BUSY = 11,
    SYSDESC_TYPE_CALL_GATE = 12,
    SYSDESC_TYPE_INT_GATE = 14,
    SYSDESC_TYPE_TRAP_GATE = 15 // only difference is interrupt gate clears IF
};

// Segment Descriptors and System Descriptors
typedef struct {
    u32 lim_15_0 : 16;  // Low bits of segment limit (only needed for TSS segment on x86_64)
    u32 base_23_0 : 24; // Low bits of segment base address (linear)
    u32 type : 4;       // Segment type (see STS_ constants)
                        // non-system descriptors: bit 3 = data descriptor vs code desctiptor
                        // data descriptor: bit 2 = expand-down limit (useful for stack selectors)
                        // data descriptor: bit 1 = allow writes
                        // code descriptor: bit 2 = conforming, tricky subject, allow user code to run
                        // with kernel selectors under certain circumstances
                        // code descriptor: bit 1 = allow reads as data (e.g. access inline constants)
                        // bit 0 = communicate 'has been accessed' in some way to the OS, abandoned use
    u32 s : 1;          // 0 = system descriptor, 1 = segment descriptor
    dpl dpl : 2;        // DLP (Descriptor Privilege Level); code ring allowed to use the segment
    u32 p : 1;          // Present
    u32 lim_19_16 : 4;  // High bits of segment limit
    u32 avail : 1;
    u32 lm : 1;         // 64-bit code
    u32 db : 1;         // Misc, depends on type, clear in 16-bit and long mode, set in 32-bit mode
    u32 g : 1;          // Granularity: if set, limit scaled by 4K (0x1000)
    u32 base_31_24 : 8; // High bits of segment base address
    // base_32_63 : 32; // System descriptor additional high bits of segment base address
    // rsrvz : 32;      // System descriptor reserved
} __attribute__((packed)) segdesc;

// 'lgdt' operand format
typedef struct {
    u16 lenm1; // length minus 1
    u64 addr;
} __attribute__((packed)) GDTR;

// Normal segment descriptor
#define SEG(type, base, lim, dpl) (struct segdesc)    \
{   ((lim) >> 12) & 0xffff, (uint)(base) & 0xffff,      \
    ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
    (uint)(lim) >> 28, 0, 0, 1, 1, (uint)(base) >> 24 }
// 16-bit segment descriptor
#define SEG16(type, base, lim, dpl) (struct segdesc)  \
{   (lim) & 0xffff, (uint)(base) & 0xffff,              \
    ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
    (uint)(lim) >> 16, 0, 0, 1, 0, (uint)(base) >> 24 }

#else // __ASSEMBLER__
#define SEG_KCODE (1 << 3)  // kernel code
#define SEG_KDATA (2 << 3)  // kernel data+stack
#define SEG_UCODE (3 << 3)  // user code
#define SEG_UDATA (4 << 3)  // user data+stack
#define SEG_TSS   (5 << 3)  // this process's task state

#define DPL_KERNEL 0
#define DPL_USER   3

#define SEG_ASM(type,dpl)                             \
        .word 0, 0;                                   \
        .byte 0, ((type) | 0x10 | (dpl) << 5 | 0x80), \
              ((type & STA_X) << 2), 0
#endif // __ASSEMBLER__

#define STA_X      0x8       // Executable segment
#define STA_W      0x2       // Writeable (non-executable segments)
#define STA_R      0x2       // Readable (executable segments)

#define SEG_NULLASM                                   \
        .word 0, 0;                                   \
        .byte 0, 0, 0, 0

#define SEG32_ASM(type,dpl)                           \
        .word 0xFFFF, 0;                              \
        .byte 0, ((type) | 0x10 | (dpl) << 5 | 0x80), \
              0xCF, 0

// segment registers, as well as TR, have visible and invisible parts
