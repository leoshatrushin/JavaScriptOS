#pragma once
#include "types.h"

// ACPI - Advanced Configuration and Power Interface
// latest spec as of august 2024:
// https://uefi.org/sites/default/files/resources/ACPI_Spec_6_5_Aug29.pdf
// Power management and configuration standard for the PC
// Allows OSes to control many different hardware features
// e.g. amount of power, thermal zones, fan control, IRQs, battery levels, etc.
// Also need to access the ACPI tables to read the IO-APIC information, used to receive hardware interrupts

// RSDP - Root System Description Pointer
// Pointer to RSDT (Root System Description Table)
// Full structure depends on whether ACPI 1 or 2 is used, 2 is backwards compatible
// With multiboot2, look for a tag with type MULTIBOOT_TAG_TYPE_ACPI_OLD or MULTIBOOT_TAG_TYPE_ACPI_NEW

// note - all addresses are physical addresses

// each SDT (System Description Table) in the XSDT has a header and specific fields following it
typedef struct {
    char signature[4]; // e.g. "APIC", not null terminated
    u32 length; // including header
    u8 revision; // table-specific revision
    u8 checksum; // sum of all bytes in table must be 0
    char oemid[6];
    char oemTableId[8];
    u32 oemRevision;
    u32 creatorId;
    u32 creatorRevision;
} __attribute__((packed)) SDTHeader;

typedef struct {
    SDTHeader sdtHeader; // signature "XSDT"
    SDTHeader sdts[];
} __attribute__((packed)) XSDT;

// Root System Description Pointer
typedef struct {
    char signature[8]; // "RSD PTR ", not null terminated
    u8 checksum; // sum of all bytes in the table must be 0
    char oemid[6];
    u8 revision;
    u32 rsdtAddress;
    // ACPI 2.0+
    u32 length;
    XSDT *xsdt; // XSDT address, if nonzero RSDT must be ignored
    u8 extendedChecksum;
    u8 reserved[3];
} __attribute__((packed)) RSDP;

// e.g. RSDP and XSDP SDTs have this structure
/*typedef struct {*/
/*    SDTHeader sdtHeader; // signature "RSDP"*/
/*    u32 sdtAddresses[]; // pointers to other SDTs*/
/*} __attribute__((packed)) RSDP;*/

typedef struct {
    SDTHeader sdtHeader; // signature "HPET"
    u32 event_timer_block_id;
    u32 reserved; // actually some type information describing the address space of the HPET registers
                   // in the ACPI table this field is the first part of a 'generic address structure'
                   // but we can safely ignore this info because the HPET spec requires the registers to be
                   // memory-mapped (thus in memory space)
    unsigned long address; // physical address of HPET registers
    u8 id; // other fields explained in HPET spec
    u16 min_ticks;
    u8 page_protection;
} __attribute__((packed)) HPET_SDT;

// Multiple APIC Description Table
typedef struct {
    SDTHeader header;
    u32 lapic_base;
    u32 flags;
    u8 descriptons[];
} __attribute__((packed)) MADT;

typedef struct {
    u8 type;
    u8 length;
} __attribute__((packed)) MADT_desc_header;

typedef struct {
    u8 ioapic_id;
    u8 rsrv;
    u32 addr;
    u32 global_system_interrupt_base;
} __attribute__((packed)) MADT_IOAPIC_desc;

typedef struct {
    u8 bus_source; // 0 = ISA
    u8 irq_source; // bus-relative interrupt source IRQ pin
    u32 global_system_interrupt; // target IRQ on APIC that this bus-relative source will signal
    u16 flags;
} __attribute__((packed)) MADT_interrupt_source_override_desc;
