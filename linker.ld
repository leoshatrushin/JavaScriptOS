ENTRY(multiboot2_start)

SECTIONS
{
    . = 1M;

    PROVIDE(PA_KERNEL_BEGIN = .);
    # keep in sync with memlayout.h
    PROVIDE(KVA_KERNEL_BEGIN = 0xFFFFFFFF80000000);

    # must be within the first 32768 bytes of the kernel image
    .multiboot2.header :
    {
        KEEP(*multiboot2_header.o(.data))
    }
    .multiboot2.text :
    {
        /* assembly stub/boot shim to get into long mode */
        *(.multiboot2.text)
    }
    .page_tables :
    {
        *(.page_tables)
    }

    . += KVA_KERNEL_BEGIN;

    .text ALIGN(4K) : AT(ADDR (.text) - KVA_KERNEL_BEGIN)
    {
        *(.text*)
    }
    .rodata ALIGN(4K) : AT(ADDR (.rodata) - KVA_KERNEL_BEGIN)
    {
        *(.rodata*)
    }

    PROVIDE(KVA_KERNEL_DATA_BEGIN = .);
    PROVIDE(PA_KERNEL_DATA_BEGIN = . - KVA_KERNEL_BEGIN);

    .data ALIGN(4K) : AT(ADDR (.data) - KVA_KERNEL_BEGIN)
    {
        *(.data*)
    }
    .bss ALIGN(4K) : AT(ADDR (.bss) - KVA_KERNEL_BEGIN)
    {
        *(COMMON)
        *(.bss*)
    }

    PROVIDE(KVA_KERNEL_END = .);
    PROVIDE(PA_KERNEL_END = . - KVA_KERNEL_BEGIN);

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack)
    }
}
