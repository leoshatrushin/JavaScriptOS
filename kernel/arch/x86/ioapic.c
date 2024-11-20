#include "acpi.h"
#include "ioapic.h"
#include "idt.h"

mmap_ioapic_t *mmio_ioapic_base;

// The actual IOAPIC registers are hidden behind a layer of indirection through the two memory-mapped
// 'command and data' registers
u32 ioapic_read_register(u8 offset) {
    mmio_ioapic_base->ioregsel = offset;
    return mmio_ioapic_base->iowin;
}

void ioapic_write_register(u8 offset, u32 value) {
    mmio_ioapic_base->ioregsel = offset;
    mmio_ioapic_base->iowin = value;
}

void init_ioapic() {
    // get the IOAPIC's base address from the MADT System Descriptor Table
    MADT_desc_header *ioapicdesc_header;
    MADT_IOAPIC_desc *ioapicdesc = (MADT_IOAPIC_desc *)ioapicdesc_header + sizeof(MADT_desc_header);
    mmio_ioapic_base = P2V((uintptr_t)ioapicdesc->addr);
    // first interrupt the IOAPIC routes
    u32 minintr = ioapicdesc->global_system_interrupt_base;

    // get maximum number of interrupts supported by the IOAPIC
    ioapicver_t ioapicver = { .raw = ioapic_read_register(IOAPIC_VER_REG_OFFSET) };
    u32 maxirq = ioapicver.max_redir_entry + 1;

    // mark all interrupts edge-triggered, active high, masked, and not routed to any CPUs
    ioapic_redir_entry_lo redir_entry_lo = {
        .polarity = PIN_POLARITY_ACTIVE_HIGH,
        .trigger_mode = TRIGGER_MODE_EDGE_TRIGGERED,
        .mask = 1,
    };
    ioapic_redir_entry_hi redir_entry_hi = {
        .destination = 0,
    };
    for (int irq = 0; irq < maxirq; ++irq){
        redir_entry_lo.vector = IRQ_INTERRUPT_OFFSET + irq; // offset IRQs to not conflict w/ CPU exceptions
        ioapic_write_register(IOAPIC_REDIRECT_TABLE_OFFSET + 2 * irq, redir_entry_lo.raw);
        ioapic_write_register(IOAPIC_REDIRECT_TABLE_OFFSET + 2 * irq + 1, redir_entry_hi.raw);
    }
}

// route irq to interrupt IRQ0_OFFSET + irq on the given lapicid (cpu)
void ioapic_route(u8 irq, u32 lapicid, boolean masked) {
    ioapic_redir_entry_lo redir_entry_lo = {
        .vector = IRQ_INTERRUPT_OFFSET + irq,
        .delivery_mode = 0, // fixed
        .destination_mode = DEST_MODE_PHYSICAL,
        .polarity = PIN_POLARITY_ACTIVE_HIGH,
        .trigger_mode = TRIGGER_MODE_EDGE_TRIGGERED,
        .mask = masked,
    };
    ioapic_redir_entry_hi redir_entry_hi = {
        .destination = lapicid,
    };
    ioapic_write_register(IOAPIC_REDIRECT_TABLE_OFFSET + 2 * irq, redir_entry_lo.raw);
    ioapic_write_register(IOAPIC_REDIRECT_TABLE_OFFSET + 2 * irq + 1, redir_entry_hi.raw);
}

void ioapic_setmask(u8 irq, boolean mask) {
    ioapic_redir_entry_lo redir_entry_lo = {
        .raw = ioapic_read_register(IOAPIC_REDIRECT_TABLE_OFFSET + 2 * irq)
    };
    redir_entry_lo.mask = mask;
    ioapic_write_register(IOAPIC_REDIRECT_TABLE_OFFSET + 2 * irq, redir_entry_lo.raw);
}
