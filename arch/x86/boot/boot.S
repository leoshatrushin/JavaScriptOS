#include "memlayout.h"
#include "segmentation.h"
#include "paging.h"
#include "x86.h"
#include "msr.h"

.equ STACK_SIZE, 0x4000

#define PTE(pt, i) ((pt) + 8 * (i))

.equ SMALL, 0
.equ LARGE, 1
.macro MAPPAGE pt, idx, addr, largepage
    movl $\addr, %eax
    .if \largepage
        orl $(PTE_P | PTE_W | PTE_PS), %eax
    .else
        orl $(PTE_P | PTE_W), %eax
    .endif
    movl %eax, \pt + 8 * \idx
.endm

.section .multiboot2.text, "ax", @progbits
.code32
.globl multiboot2_start
multiboot2_start:
    movl %ebx, %edi # address of multiboot2 information structure
    movl %eax, %esi # magic number

    # intel manual, 10.8.5 Initializing IA-32e Mode: "The MOV CR0 instruction that enables paging and the following
    # instructions must be located in an identity-mapped page"
    MAPPAGE BOOT_PML4,      0, BOOT_PDPT_id, SMALL
    MAPPAGE BOOT_PDPT_id,   0, BOOT_PD_id,   SMALL
    MAPPAGE BOOT_PD_id,     0, 0,            LARGE
    # 0xffffffff80000000 -> 511, 510, 0, 0
    MAPPAGE BOOT_PML4,    511, BOOT_PDPT_hh, SMALL
    MAPPAGE BOOT_PDPT_hh, 510, BOOT_PD_id,   SMALL

    # see intel manual 10.8.5 Initializing IA-32e Mode
    # enable physical address extension
    movl %cr4, %eax
    or $CR4_PAE, %eax
    movl %eax, %cr4

    # enable long mode
    mov $MSR_EFER, %ecx
    rdmsr
    or $MSR_EFER_LONG_MODE, %eax
    wrmsr

    # cr3 = BOOT_PML4
    movl $BOOT_PML4, %eax
    movl %eax, %cr3

    # enable paging and write protection
    movl %cr0, %eax
    or $(CR0_PG | CR0_WP), %eax
    movl %eax, %cr0

    # load GDT
    lgdt KV2P(gdtdesc_pa)
    ljmpl $0x08, $KV2P(kernel_jumper)

.code64
.section .text
kernel_jumper:
    # now paging is truly enabled
    # even for the gdtdesc base address
    lgdt gdtdesc_va

    # update segment selectors
    movw $SEG_KDATA, %ax
    movw %ss, %ax
    movw %ds, %ax
    movw %es, %ax
    movw %fs, %ax
    movw %gs, %ax

    # switch to executing at the higher half
    movq $kernel_main, %rax
    jmp *%rax

.section .page_tables, "aw", @nobits

.balign PGSIZE
BOOT_PML4:
    .skip PGSIZE
BOOT_PDPT_id:
    .skip PGSIZE
BOOT_PD_id:
    .skip PGSIZE
BOOT_PDPT_hh:
    .skip PGSIZE

.section .data
gdt64:
    SEG_NULLASM
    SEG_ASM(STA_X | STA_R, DPL_KERNEL) # kernel code segment
    SEG_ASM(STA_W,         DPL_KERNEL) # kernel data segment
    SEG_ASM(STA_X | STA_R, DPL_USER)   # user code segment
    SEG_ASM(STA_W,         DPL_USER)   # user data segment
    SEG_NULLASM                        # tss segment low
    SEG_NULLASM                        # tss segment high
gdtdesc_pa:
    .word . - gdt64 - 1
    .quad KV2P(gdt64)
gdtdesc_va:
    .word gdtdesc_pa - gdt64 - 1
    .quad gdt64

.section .bss
stack:
    .skip STACK_SIZE
stack_top:
