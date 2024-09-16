#include "pci.h"
#include "pcidef.h"
#include "../arch/x86/asm.h"
#include "../console.h"
#include "../../libc/stdio.h"

u32 pci_config_address(u8 bus, u8 device, u8 function, u8 offset) {
    return ((pci_config_address_t) {
        .bus = bus,
        .device = device,
        .function = function,
        .offset = static_cast<u8>(offset & 0xFC),
        .enable = 1
    }).raw;
}

u8 pci_read_register8(u8 bus, u8 device, u8 function, u8 offset) {
    out32(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, device, function, offset));
    // https://wiki.osdev.org/PCI:
    // "all reads and writes must be both 32-bits and aligned to work on all implementations"
    return in32(PCI_CONFIG_DATA_PORT) >> ((offset & 0b11) * 8);
}

u16 pci_read_register16(u8 bus, u8 device, u8 function, u8 offset) {
    out32(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, device, function, offset));
    return in32(PCI_CONFIG_DATA_PORT) >> ((offset & 0b11) * 8);
}

u32 pci_read_register32(u8 bus, u8 device, u8 function, u8 offset) {
    out32(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, device, function, offset));
    return in32(PCI_CONFIG_DATA_PORT) >> ((offset & 0b11) * 8);
}

void pci_write_register8(u8 bus, u8 device, u8 function, u8 offset, u8 value) {
    out32(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, device, function, offset));
    u32 cur = in32(PCI_CONFIG_DATA_PORT);
    cur &= ~(0xFF << ((offset & 0b11) * 8));
    cur |= value << ((offset & 0b11) * 8);
    out32(PCI_CONFIG_DATA_PORT, cur);
}

void pci_write_register16(u8 bus, u8 device, u8 function, u8 offset, u16 value) {
    out32(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, device, function, offset));
    u32 val = in32(PCI_CONFIG_DATA_PORT);
    val &= ~(0xFFFF << ((offset & 0b10) * 8));
    val |= value << ((offset & 0b10) * 8);
    out32(PCI_CONFIG_DATA_PORT, val);
}

void pci_write_register32(u8 bus, u8 device, u8 function, u8 offset, u32 value) {
    out32(PCI_CONFIG_ADDRESS_PORT, pci_config_address(bus, device, function, offset));
    out32(PCI_CONFIG_DATA_PORT, value);
}

void enumerate_pci_bus(u8 bus);

void enumerate_function(u8 bus, u8 device, u8 function) {
    u16 vendorID = pci_read_register16(bus, device, function, PCI_VENDOR_ID_REG_OFFSET);
    u16 deviceID = pci_read_register16(bus, device, function, PCI_DEVICE_ID_REG_OFFSET);
    u8 revisionID = pci_read_register8(bus, device, function, PCI_REVISION_ID_REG_OFFSET);
    u16 device_class = pci_read_register8(bus, device, function, PCI_CLASS_CODE_REG_OFFSET);
    u16 subclass = pci_read_register8(bus, device, function, PCI_SUBCLASS_REG_OFFSET);
    u8 progIF = pci_read_register8(bus, device, function, PCI_PROG_IF_REG_OFFSET);
    char vendor[128], device_str[128], class_str[128], subclass_str[128];
    int n = sprintf(vendor, "%x ", vendorID);
    strcpy(vendor + n, pci_vendor_strings[vendorID]);
    n = sprintf(device_str, "%x ", deviceID);
    strcpy(device_str + n, (*pci_device_strings[vendorID])[deviceID]);
    n = sprintf(class_str, "%x ", device_class);
    strcpy(class_str + n, pci_class_strings[device_class]);
    n = sprintf(subclass_str, "%x ", subclass);
    strcpy(subclass_str + n, (*pci_subclass_strings[device_class])[subclass]);
    debug_printf("Found PCI device:\n\tbus:%u, device:%u, function:%u"
                                  "\n\tvendor:%s, device:%s, revision:%u"
                                  "\n\tclass:%s, subclass:%s, interface:%u\n",
                                  (u64)bus, (u64)device, (u64)function,
                                  vendor, device_str, (u64)revisionID,
                                  class_str, subclass_str, (u64)progIF);
    // recurse into PCI to PCI bridges
    if (device_class == BridgeDevice && subclass == PCI_TO_PCI) {
        u8 secondaryBus = pci_read_register8(bus, device, function, PCI_SECONDARY_BUS_NUM_REG_OFFSET);
        enumerate_pci_bus(secondaryBus);
    }
}

void enumerate_device(u8 bus, u8 device) {
    u16 vendorID = pci_read_register16(bus, device, 0, PCI_VENDOR_ID_REG_OFFSET);
    if (vendorID == PCI_VENDOR_ID_INVALID) return;
    u8 function = 0;
    enumerate_function(bus, device, function);
    pci_header_type_reg ht_reg = { .raw = pci_read_register8(bus, device, 0, PCI_HEADER_TYPE_REG_OFFSET)};
    // must check - some devices report details for function 0 for all functions
    if (ht_reg.is_multi_function) {
        for (u8 function = 1; function < 8; ++function) {
            vendorID = pci_read_register16(bus, device, function, PCI_VENDOR_ID_REG_OFFSET);
            if (vendorID == PCI_VENDOR_ID_INVALID) continue;
            enumerate_function(bus, device, function);
        }
    }
}

void enumerate_pci_bus(u8 bus) {
    static u8 enumerated_buses[8];
    if (enumerated_buses[bus]) return;
    debug_printf("Enumerating PCI bus %u\n", (u64)bus);
    enumerated_buses[bus] = 1;
    for (u8 device = 0; device < 32; ++device) {
        enumerate_device(bus, device);
    }
}

// rely on firmware to configure PCI buses properly (setting up PCI to PCI bridges to forward requests
// from one bus to another)
void enumerate_all_pci_buses() {
    // bus 0 is always present, it is the root bus connected directly to the CPU, i.e. the host bridge
    pci_header_type_reg bus0_ht = { .raw = pci_read_register8(0, 0, 0, PCI_HEADER_TYPE_REG_OFFSET) };
    if (!bus0_ht.is_multi_function) {
        // only one PCI host controller at (0, 0, 0), responsible for bus 0
        enumerate_pci_bus(0);
    } else {
        // multiple PCI host controllers at (0, 0, X), responsible for bus X
        // catch any buses not reachable through recursive PCI to PCI bridges from bus 0
        for (u8 bus_as_function_number = 0; bus_as_function_number < 8; ++bus_as_function_number) {
            debug_printf("Checking PCI bus %u\n", (u64)bus_as_function_number);
            u16 vendorID = pci_read_register16(0, 0, bus_as_function_number, PCI_VENDOR_ID_REG_OFFSET);
            if (vendorID == PCI_VENDOR_ID_INVALID) {
                debug_printf("No PCI host controller at (0, 0, %u)\n", (u32)bus_as_function_number);
            }
            enumerate_pci_bus(bus_as_function_number);
        }
    }
    debug_printf("PCI enumeration complete\n");
}

struct {void* base; size_t size;} get_base(u8 bus, u8 device, u8 function) {
}

void enable_msix(u8 bus, u8 device, u8 function) {
    // check if device supports extended capabilities list
    pci_status_reg status = { .raw = pci_read_register16(bus, device, function, PCI_STATUS_REG_OFFSET) };
    if (!status.capabilities_list) {
        debug_printf("Error: No capabilities list\n");
        return;
    }

    // find MSI-X capability in capabilities list
    u8 cap_off = pci_read_register8(bus, device, function, PCI_CAPABILITIES_POINTER_REG_OFFSET);
    boolean found_msix_cap = 0;
    while (cap_off) {
        pci_capaibility_common cap = { .raw = pci_read_register16(bus, device, function, cap_off) };
        if (cap.capability_id == MSIX) {
            found_msix_cap = 1;
            break;
        }
        cap_off = cap.next_capability_pointer;
    }
    if (!found_msix_cap) {
        debug_printf("Error: No MSI-X capability\n");
        return;
    }

    // find 
    u8 msix_bar_off = PCI_BAR0_OFFSET + pci_read_register8(bus, device, function, cap_off + MSIX_BIR_OFFSET);
    u32 msix_bar_v = pci_read_register32(bus, device, function, msix_bar_off);
    if (BAR_REG_IS_IO_SPACE(msix_bar_v)) {
        debug_printf("Error: MSI-X BAR is in IO space\n");
        return;
    }
    memory_space_bar_reg msix_bar = { .raw = msix_bar_v };
    if (!(msix_bar.type == MEMORY_SPACE_BAR_64_BIT)) {
        debug_printf("Error: MSI-X BAR is not 64-bit\n");
        return;
    }
    pci_command_reg disable_decode = { .io_space = 1 };
}
