#pragma once
#include "../../../libk/types.h"

namespace Kernel::AHCI {

constexpr u8 AHCI_Base_Memory_Register_BARNO = 5; // ABAR

volatile union global_host_control_reg {
    struct {
    u8 hba_reset : 1;
    u8 interrupt_enable : 1;
    u8 msi_revert_to_single_message : 1;
    u32 rsrv : 28;
    u8 ahci_enable : 1;
    } __attribute__((packed));
    u32 raw;
};

volatile struct [[gnu::packed]] GenericHostControl {
    u32 host_capabilities; // capacity?
    global_host_control_reg global_host_control;
    u32 interrupt_status; 
    u32 ports_implemented;
    u32 version;
    u32 ccc_ctrl;    /* Command Completion Coalescing Control */
    u32 ccc_ports;   /* Command Completion Coalsecing Ports */
    u32 em_location; /* Enclosure Management Location */
    u32 em_ctrl;     /* Enclosure Management Control */
    u32 extended_host_capabilities;        /* Host Capabilities Extended */
    u32 bohc;        /* BIOS/OS Handoff Control and Status */
};
static_assert(sizeof(GenericHostControl) == 0x2c);

volatile struct [[gnu::packed]] PortControl {
    u32 command_list_base_lower;
    u32 command_list_base_upper;
    u32 fis_base_lower; // 256-byte aligned
    u32 fis_base_upper;
    u32 interrupt_status;
    u32 interrupt_enable;
    u32 cmd_and_status;
    u32 rsrv;
    u32 task_file_data;
    u32 signature;
    u32 sata_status;  // SCR0:SStatus
    u32 sata_control; // SCR2:SControl
    u32 sata_error;   // SCR1:SError
    u32 sata_active;  // SCR3:SActive
    u32 command_issue;
    u32 sata_notification; // SCR4:SNotification
    u32 fis_switch_control;
    u32 device_sleep;
    u8 rsrv2[0x70 - 0x48];
    u8 vendor_specific[0x80 - 0x70];
};
static_assert(sizeof(PortControl) == 0x80);

// This memory area should be configured as uncacheable as memory-mapped hardware registers are not normal
// prefetchable RAM
// For the same reason, the data structures are marked volatile
volatile struct [[gnu::packed]] HBARegisters {
    GenericHostControl control_regs;
    u8 rsrv[52];
    u8 nvmhci[64];
    u8 vendor_specific[0x100-0xA0];
    PortControl port_regs[]; // 1-32 ports, each one can attach a single SATA device
};
static_assert(sizeof(HBARegisters) == 0x100);
static_assert(__builtin_offsetof(HBARegisters, port_regs[32]) == 0x100 + 0x80 * 32);

}
