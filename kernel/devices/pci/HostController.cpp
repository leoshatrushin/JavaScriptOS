#include <bitset>
#include <kernel/devices/pci/HostController.hpp>
#include <kernel/devices/pci/Device.hpp>
#include "Allocator.hpp"

using namespace Kernel::PCI;

#define co_yield_from(expr) co_yield std::ranges::elements_of(expr)

HostController::device_generator enumerate_pci_bus(u8 bus); void enable_msix(u8 bus, u8 device, u8 function);

HostController::device_generator enumerate_function(u8 bus, u8 deviceno, u8 function) {
    DeviceBase device{bus, deviceno, function};
    co_yield device;

    // recurse into PCI to PCI bridges
    if (device.class_id() == DeviceClass::BridgeDevice
            && device.subclass_id() == (u8)Bridge::SubclassID::PCI_TO_PCI) {
        auto bridge = PCIToPCIBridge(bus, deviceno, function);
        co_yield_from(enumerate_pci_bus(bridge.secondary_bus()));
    }
}

HostController::device_generator enumerate_device(u8 bus, u8 deviceno) {
    auto device = DeviceFactory::create(bus, deviceno, 0);
    if (device.vendor_id() == VendorID::DeviceNotPresent) co_return;
    u8 function = 0;
    co_yield_from(enumerate_function(bus, deviceno, function));
    // must check - some devices report details for function 0 for all functions
    if (device.header_type()->is_multi_function) {
        for (u8 function = 1; function < 8; ++function) {
            device = DeviceFactory::create(bus, deviceno, function);
            if (device.vendor_id() == VendorID::DeviceNotPresent) continue;
            co_yield_from(enumerate_function(bus, deviceno, function));
        }
    }
}

HostController::device_generator enumerate_pci_bus(u8 bus) {
    static std::bitset<256> enumerated_buses;
    if (enumerated_buses[bus]) co_return;
    debug_printf("Enumerating PCI bus %u\n", (u64)bus);
    enumerated_buses[bus] = 1;
    for (u8 device = 0; device < 32; ++device) {
        co_yield_from(enumerate_device(bus, device));
    }
}

HostController::device_generator HostController::attached_devices() {
    // rely on firmware to configure PCI buses properly (setting up PCI to PCI bridges to forward requests
    // from one bus to another)
    // Systems must provide a mechanism that allows software to generate PCI configuration transactions
    // This mechanism is typically located in the host bridge
    // This spec specifies the mechanism for PC-AT compatible systems
    // Main host bridge/controller at (0, 0, 0)
    auto device = DeviceFactory::create(0, 0, 0);
    if (!device.header_type()->is_multi_function) {
        co_yield_from(enumerate_pci_bus(0));
    } else {
        // Other PCI host bridges at (0, 0, X), responsible for bus X
        for (u8 bus_as_function_number = 0; bus_as_function_number < 8; ++bus_as_function_number) {
            device = DeviceFactory::create(0, 0, bus_as_function_number);
            debug_printf("Checking PCI bus %u\n", (u64)bus_as_function_number);
            if (device.vendor_id() == PCI::VendorID::DeviceNotPresent)
                debug_printf("No PCI host bridge at (0, 0, %u)\n", (u32)bus_as_function_number);
            /*co_yield_from(enumerate_pci_bus(bus_as_function_number));*/
        }
    }
    debug_printf("PCI enumeration complete\n");
    co_return;
}
