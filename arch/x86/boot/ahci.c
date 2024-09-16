// An AHCI controller (referred to as host bus adapter, or HBA) is designed to be a data movement engine
// between system memory and SATA devices
// It provides a standard PCI interface + bus master capabilities to the host

// latest spec as of august 2024:
// https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf

#include "pci.h"
#include "console.h"

void init_ahci(u8 bus, u8 device, u8 function) {
    pci_status_reg ahci_status = { .raw = pci_read_register16(bus, device, function, PCI_STATUS_REG_OFFSET)};
    if (!ahci_status.capabilities_list) {
        debug_printf("Error: AHCI controller does not have a capabilities list\n");
        return;
    }

    
}
