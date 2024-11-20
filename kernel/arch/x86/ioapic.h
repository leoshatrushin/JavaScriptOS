// intel 82093AA I/O APIC datasheet
// https://web.archive.org/web/20161130153145/http://download.intel.com/design/chipsets/datashts/29056601.pdf
#pragma once
#include "types.h"
#include "acpi.h"

// Note - alternative to MMIO is called PMIO (port-mapped IO)

// ioregsel command register and iowin data register used to access other IOAPIC registers
typedef struct {
    u8 ioregsel;
    u8 rsrv[7];
    u32 iowin;
} __attribute__((packed)) mmap_ioapic_t;

#define IOAPIC_ID_REG_OFFSET         0x00 // R/W
#define IOAPIC_VER_REG_OFFSET        0x01 // RO
#define IOAPIC_ARB_REG_OFFSET        0x02 // RO
#define IOAPIC_REDIRECT_TABLE_OFFSET 0x10 // R/W, 64-bits redirection entries (two adjacent ioregsel accesses)

typedef union {
    struct {
    u32 rsrv : 24;
    u8 id : 4;
    u8 rsrv2 : 4;
    };
    u32 raw;
} __attribute__((packed)) ioapicid_t;

typedef union {
    struct {
    u8 version;
    u8 rsrv;
    u8 max_redir_entry; // number of interrupt pins minus 1
    u8 rsrv2;
    };
    u32 raw;
} __attribute__((packed)) ioapicver_t;

typedef union {
    struct {
    u32 rsrv : 24;
    u8 arbitration_id : 4;
    u8 rsrv2 : 4;
    };
    u32 raw;
} __attribute__((packed)) ioapicarb_t;

enum dest_mode {
    DEST_MODE_PHYSICAL = 0,
    DEST_MODE_LOGICAL = 1,
};

enum pin_polarity {
    PIN_POLARITY_ACTIVE_HIGH = 0,
    PIN_POLARITY_ACTIVE_LOW = 1,
};

enum trigger_mode_t {
    TRIGGER_MODE_LEVEL_TRIGGERED = 0,
    TRIGGER_MODE_EDGE_TRIGGERED = 1,
};

typedef union {
    struct {
    u32 vector : 8; // 16-254
    u32 delivery_mode : 3; // ?
    enum dest_mode destination_mode : 1; // 0: physical, 1: logical
    u32 delivery_status : 1; // RO, 0: idle, 1: send pending
    enum pin_polarity polarity : 1; // pin polarity
    u32 remote_irr : 1; // RO, set when a LAPIC accepts a level-triggered interrupt
                        // cleared on EOI message with a matching interrupt vector
    enum trigger_mode_t trigger_mode : 1;
    u32 mask : 1; // note - may have already been accepted by a LAPIC but not yet dispensed to the processor
    u32 rsrv : 15;
    };
    u64 raw;
} __attribute__((packed)) ioapic_redir_entry_lo;

typedef union {
    struct {
    u32 rsrv : 24;
    u8 destination; // physical mode: APIC ID, logical mode: processor set
    };
    u32 raw;
} __attribute__((packed)) ioapic_redir_entry_hi;

void init_ioapic();
void ioapic_route(u8 irq, u32 lapicid, boolean masked);
void ioapic_setmask(u8 irq, boolean mask);
