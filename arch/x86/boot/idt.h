#pragma once
#include "types.h"
#include "segmentation.h"

typedef void* isr;

// interrupt sequence
// - CPU finds idtdesc IDT[interrupt_number]
// - TSS descriptor is cached in TR
// - if idtdesc->cs.dpl = X > CPL, cpu switches to stack tss->rspX
// - CPU pushes iret frame (with the old rsp, saved internally)
// - CPU switches cs to idtdesc->cs
// - CPU jumps to idtdesc->offset interrupt handler
// - interrupt handler runs on the new stack

// Interrupt Descriptor Table
typedef struct {
    u16               offset_low; // must be in canonical form
    segsel            cs; // must be a 64-bit segment
    u8                ist : 2; // Interrupt Stack Table - used with TSS to switch stacks
                          // useful for edge cases like NMIs
    u8                zero : 6;
    enum sysdesc_type type : 4;
    u8                zero2 : 1;
    u8                dpl : 2; // Descriptor Privilege Level
    u8                p : 1;
    u16               offset_mid;
    u32               offset_high;
    u32               rsrv;
} __attribute__((__packed__)) idtdesc;

// 'lidt' operand format
typedef struct IDT_reg {
    u16    lenm1; // length minus 1
    size_t offset;
} __attribute__((__packed__)) IDTR;

// built on the stack by the hardware and alltraps.S, and passed to trap()
typedef struct {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;

    u16 gs;
    u16 fs;
    u16 es;
    u16 ds;
    
    u64 interrupt_num;
    u64 error_code;
    
    // "iret frame" (plus possibly error code)
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __attribute__((__packed__)) trapframe;

#define DIVIDE_ERROR              0
#define DEBUG_EXC                 1
#define NMI_INTERRUPT             2
#define BREAKPOINT                3
#define OVERFLOW                  4
#define BOUND_RANGE_EXCEED        5
#define INVALID_OPCODE            6
#define DEV_NOT_AVL               7
#define DOUBLE_FAULT              8
#define COPROC_SEG_OVERRUN        9
#define INVALID_TSS              10
#define SEGMENT_NOT_PRESENT      11
#define STACK_SEGMENT_FAULT      12
#define GENERAL_PROTECTION       13
#define PAGE_FAULT               14
#define INT_RSV                  15
#define FLOATING_POINT_ERR       16
#define ALIGNMENT_CHECK          17
#define MACHINE_CHECK            18
#define SIMD_FP_EXC              19

#define APIC_TIMER_INTERRUPT_NUM     32
#define APIC_SPURIOUS_INTERRUPT_NUM 255

#define IRQ_INTERRUPT_OFFSET              32
// defined in the chipset spec
// https://wiki.qemu.org/Features/Q35
// qemu-system-x86_64 (default): Intel PIIX4
// qemu-system-x86_64 -machine q35: Intel ICH9
// https://www.intel.sg/content/dam/doc/datasheet/io-controller-hub-9-datasheet.pdf
// section 5.8 8259 Interrupt Controllers (PIC)
#define IRQ_LAPIC_TIMER         0
#define IRQ_KEYBOARD            1
#define IRQ_PIT                 2
#define IRQ_COM2                3 // + COM4, but idk where it says that in the spec or what "COM" even means
#define IRQ_COM1                4 // + COM3, see https://wiki.osdev.org/Serial_Ports
                                  // idk where to find "there exists a serial port device on this chipset"
                                  // in the spec
#define KEYBOARD_INTERRUPT_NUM       33
#define PIT_INTERRUPT_NUM            34


#define SYSCALL_INTERRUPT_NUM        0x80 // used on Linux


void init_idt();
void load_idt();
