// Message Signaled Interrupts
// See:
// - PCI local bus specification
// - Intel manual section
#pragma once
#include "types.h"

// MSI interrupts seem to be invariably edge-triggered active high
typedef struct {
    u8 rsrv0z : 2;
    u8 destination_mode : 1; // 0 = physical (LAPIC id), 1 = logical (processor set)
    u8 redirection_hint : 1; // idk
    u8 rsrv0;
    u8 destination_id;
    u16 _0fee : 12;
} __attribute__((packed)) x86_64_msi_message_address_reg;
