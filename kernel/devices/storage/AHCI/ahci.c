#include "pci.h"
#include "console.h"

void init_ahci(u8 bus, u8 device, u8 function) {
    pci_status_reg ahci_status = { .raw = pci_read_register16(bus, device, function, PCI_STATUS_REG_OFFSET)};
    if (!ahci_status.capabilities_list) {
        debug_printf("Error: AHCI controller does not have a capabilities list\n");
        return;
    }

    
}
