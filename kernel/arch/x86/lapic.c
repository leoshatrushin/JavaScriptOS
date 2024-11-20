#include "x86.h"
#include "asm.h"
#include "msr.h"
#include "idt.h"
#include "lapic.h"

boolean using_x2apic_mode;
void *apic_mmio_base;

void disable_pic(void);

void init_lapic() {
    // check for x2apic mode support
    u32 _, ecx, edx;
    cpuid(1, 0, &_, &_, &ecx, &edx);
    u64 apic_base_v = rdmsr(MSR_IA32_APIC_BASE);
    msr_ia32_apic_base_t apic_base = *(msr_ia32_apic_base_t *)&apic_base_v;
    if (!(ecx & CPUID_01_ECX_X2APIC)) {
        using_x2apic_mode = true;
        // enable x2apic mode
        apic_base.x2apic_enable = 1;
        wrmsr(MSR_IA32_APIC_BASE, apic_base_v);
    } else if (ecx & CPUID_01_EDX_APIC) {
        using_x2apic_mode = false;
        apic_mmio_base = (void*)(uintptr_t)apic_base.mmio_base;
    } else {
        // not good
        return;
    }

    // enable apic and set spurious interrupt vector
    lapic_spurious_interrupt_vector_reg_t sivr = {
        .vector = APIC_SPURIOUS_INTERRUPT_NUM,
        .apic_software_enable = 1
    };
    lapic_write_register(LAPIC_SPURIOUS_INTERRUPT_VECTOR_REG_OFFSET, *(u32*)&sivr);

    disable_pic();
}

u32 lapid_read_register(u32 offset) {
    if (using_x2apic_mode)
        return rdmsr(0x800 + (offset >> 4));
    else
        return *(u32*)(apic_mmio_base + offset);
}

void lapic_write_register(u32 offset, u32 value) {
    if (using_x2apic_mode)
        wrmsr(0x800 + (offset >> 4), value);
    else
        *(u32*)(apic_mmio_base + offset) = value;
}

u32 lapicid() {
    if (using_x2apic_mode) {
        return lapid_read_register(LAPIC_ID_REG_OFFSET);
    } else {
        u32 id_v = lapid_read_register(LAPIC_ID_REG_OFFSET);
        lapic_xapic_id_reg_t id = *(lapic_xapic_id_reg_t *)&id_v;
        return id.lapic_id;
    }
}

// must be called by interrupt handlers before iret
void lapic_eoi() {
    lapic_write_register(LAPIC_EOI_REG_OFFSET, 0);
}

boolean lapic_is_x2apic_mode() {
    return using_x2apic_mode;
}

#define PIC1_DATA_PORT         0x20    // Master (IRQs 0-7)
#define PIC2_DATA_PORT         0xA0    // Slave (IRQs 8-15)

void disable_pic(void) {
    // mask all interrupts
    outb(PIC1_DATA_PORT, 0xFF);
    outb(PIC2_DATA_PORT, 0xFF);
}
