#include "tss.h"
#include "segmentation.h"
#include "asm.h"

extern segdesc gdt64[];
extern u64 stack[];

tssseg_t kernel_tss;

void initialize_tss() {
    // every thread will have it's own rsp0 pointer
    kernel_tss.rsp0 = (u64)stack + 16384;
}

void load_tss() {
    gdt64[SEGIDX_TSS_LOW] = (segdesc){
        .lim_15_0 = (u16)sizeof(kernel_tss),
        .base_23_0 = (uintptr_t)&kernel_tss & 0xFFFFFF,
        .type = SYSDESC_TYPE_TSS_AVAIL,
        .p = 1,
        .lim_19_16 = (u8)(sizeof(kernel_tss) >> 16),
        .base_31_24 = (u8)((uintptr_t)&kernel_tss >> 24)
    };
    u64 base_63_32 = (u32)((uintptr_t)&kernel_tss >> 32);
    gdt64[SEGIDX_TSS_HIGH] = *(segdesc*)&base_63_32;

    segsel tss_sel = SEGSEL_TSS_LOW;
    ltr(*(u16*)&tss_sel);
}
