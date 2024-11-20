#include "segmentation.h"
#include "idt.h"
#include "asm.h"
#include "x86.h"

// note - would be cool to initialize this at link time (when isr addresses are known)
// it seems ld linker scripts don't support constructs like loops
// maybe would be cool to have a linker able to communicate with a python script
extern isr vectors[];
idtdesc idt[N_VECTORS];

void setidtdesc(idtdesc *gate, idtdesc_type type, segsel cs, isr off, dpl dpl) {
    gate->offset_low = (uintptr_t)off & 0xffff;
    gate->cs = cs;
    gate->type = type;
    gate->dpl = dpl;
    gate->p = 1;
    gate->offset_mid = (uintptr_t)off >> 16;
    gate->offset_high = (uintptr_t)off >> 32;
}

void init_idt() {
    for (int i = 0; i < N_VECTORS; i++)
        setidtdesc(&idt[i],         IDT_TYPE_INT_GATE,  SEG_KCODE, vectors[i],         DPL_KERNEL);
    setidtdesc(    &idt[T_SYSCALL], IDT_TYPE_TRAP_GATE, SEG_KCODE, vectors[T_SYSCALL], DPL_USER);
}

void load_idt() {
    lidt(idt, sizeof(idt));
}
