#include "pci.hpp"
#include "Definitions.hpp"
#include "../console.h"
#include "../../libc/stdio.h"
#include "../libcppk/algorithm.hpp"
#include "../arch/x86/asm.h"

void enumerate_pci_bus(u8 bus); void enable_msix(u8 bus, u8 device, u8 function);

using namespace PCI;

void enumerate_function(u8 bus, u8 device, u8 function) {
    auto device_cfg = DeviceFactory::create(bus, device, function);

    auto vendor_id   = device_cfg.vendor_id().cast();
    auto device_id   = device_cfg.device_id().cast();
    auto class_code  = device_cfg.class_code().cast();
    auto subclass    = device_cfg.subclass().cast();
    auto revision_id = device_cfg.revision_id().cast();
    auto prog_if     = device_cfg.prog_if().cast();

    // TODO: maybe change PCI string getting mechanism to not require ugly underlying casts
    char vendor_str[128], device_str[128], class_str[128], subclass_str[128];
    int n;
    n = sprintf(vendor_str, "%x ", static_cast<u64>(vendor_id));
    strcpy(vendor_str + n, vendor_strings[(int)vendor_id]);
    n = sprintf(device_str, "%x ", static_cast<u64>(device_id));
    strcpy(device_str + n, (*device_strings[(int)vendor_id])[device_id]);
    n = sprintf(class_str, "%x ", static_cast<u64>(class_code));
    strcpy(class_str + n, class_strings[(int)class_code]);
    n = sprintf(subclass_str, "%x ", static_cast<u64>(subclass));
    strcpy(subclass_str + n, (*subclass_strings[(int)class_code])[subclass]);
    debug_printf("Found PCI device:\n\tbus:%u, device:%u, function:%u"
                                  "\n\tvendor:%s, device:%s, revision:%u"
                                  "\n\tclass:%s, subclass:%s, interface:%u\n",
                                  (u64)bus, (u64)device, (u64)function,
                                  vendor_str, device_str, (u64)revision_id,
                                  class_str, subclass_str, (u64)prog_if);

    // recurse into PCI to PCI bridges
    if (class_code == DeviceClass::BridgeDevice && subclass == (u8)bridge_subclass::PCI_TO_PCI) {
        auto bridge_config_ptr = dynamic_cast<PCIToPCIBridgeConfigSpace*>(&device_cfg);
        if (!bridge_config_ptr) {
            debug_printf("Error: PCI to PCI bridge config space dynamic cast\n");
            hlt();
        }
        auto bridge_config = *bridge_config_ptr;
        enumerate_pci_bus(bridge_config.secondary_bus());
    }
}

void enumerate_device(u8 bus, u8 device) {
    auto device_cfg = PCI::PCIDeviceConfigSpaceFactory::create(bus, device, 0);
    if (device_cfg.vendor_id() == PCI::vendor_id_reg::INVALID) return;
    u8 function = 0;
    enumerate_function(bus, device, function);
    // must check - some devices report details for function 0 for all functions
    if (device_cfg.header_type()->is_multi_function) {
        for (u8 function = 1; function < 8; ++function) {
            device_cfg = PCI::PCIDeviceConfigSpaceFactory::create(bus, device, function);
            if (device_cfg.vendor_id() == PCI::vendor_id_reg::INVALID) continue;
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
    auto device_cfg = PCI::PCIDeviceConfigSpaceFactory::create(0, 0, 0);
    if (!device_cfg.header_type()->is_multi_function) {
        // only one PCI host controller at (0, 0, 0), responsible for bus 0
        // bus 0 is always present, it is the root bus connected directly to the CPU, i.e. the host bridge
        enumerate_pci_bus(0);
    } else {
        // multiple PCI host controllers at (0, 0, X), responsible for bus X
        // catch any buses not reachable through recursive PCI to PCI bridges from bus 0
        for (u8 bus_as_function_number = 0; bus_as_function_number < 8; ++bus_as_function_number) {
            debug_printf("Checking PCI bus %u\n", (u64)bus_as_function_number);
            if (device_cfg.vendor_id() == PCI::vendor_id_reg::INVALID) {
                debug_printf("No PCI host controller at (0, 0, %u)\n", (u32)bus_as_function_number);
            }
            enumerate_pci_bus(bus_as_function_number);
        }
    }
    debug_printf("PCI enumeration complete\n");
}

void enable_msix(u8 bus, u8 device, u8 function) {
    auto device_cfg_base = PCI::PCIDeviceConfigSpaceFactory::create(bus, device, function);
    auto device_cfg_ptr = dynamic_cast<PCI::PCIDeviceConfigSpace*>(&device_cfg_base);
    if (device_cfg_ptr == nullptr) {
        debug_printf("Error: Device config space dynamic cast\n");
        hlt();
    }
    auto device_cfg = *device_cfg_ptr;

    if (!device_cfg.status_reg()->has_capabilities_list) {
        debug_printf("Error: Device does not support extended capabilities list\n");
        hlt();
    }

    auto capabilities = device_cfg.capabilities();
    auto msix_cfg_it = std::find_if(capabilities.begin(), capabilities.end(), [](auto capability_cfg) {
        return capability_cfg.capability_id() == PCI::capability::MSIX;
    });
    if (msix_cfg_it == capabilities.end()) {
        debug_printf("Error: No MSI-X capability\n");
        hlt();
    }
    auto msix_cfg_base = *msix_cfg_it;
    auto msix_cfg_ptr = dynamic_cast<PCI::MSIXCapability*>(&msix_cfg_base);
    if (msix_cfg_ptr == nullptr) {
        debug_printf("Error: MSI-X config space dynamic cast\n");
        hlt();
    }
    auto msix_cfg = *msix_cfg_ptr;

    BAR_t msix_bar = device_cfg.BAR(msix_cfg.table_BIR());
    if (!BAR_IS_MEMORY_SPACE(msix_bar)) {
        debug_printf("Error: MSI-X BAR is in IO space\n");
        hlt();
    }
    memory_space_bar msix_mem = msix_bar.memory;

    /*if (!(msix_mem.type == PCI::memory_space_bar_type::_64_BIT)) {*/
    /*    debug_printf("Error: MSI-X BAR is not 64-bit\n");*/
    /*    hlt();*/
    /*}*/
    /*device_cfg.BAR(msix_cfg.table_BIR()) = { .raw = 0xFFFFFFFF };*/

    // disable IOAPIC interrupts and enable MSI-X interrupts
    device_cfg.command_reg() = { .interrupt_disable = 1 };
    msix_cfg.message_control() |= { .enable = 1 };

    msix_table_entry* msix_table_base = reinterpret_cast<msix_table_entry*>(msix_mem.base_addr);
    MemoryProxy<msix_table_entry> msix_table {msix_table_base};
    x86_64_msi_message_address_reg addr = { .destination_id = 0, ._0x0fee = 0x0FEE }; // CPU 0
    x86_64_msi_message_data_reg data = { .vector = DISK_VECTOR };
    msix_table[0] = { .message_addr = addr, .message_data = data };
}
