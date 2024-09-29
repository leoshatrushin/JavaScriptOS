// An AHCI controller is a communication engine between system memory and SATA (Serial ATA) devices
// By providing a standard PCI interface + bus master capability to the host
// Can easily access SATA devices using system memory and memory-mapped I/O, without the need for
// manipulating annoying task files like IDE
// HBA (Host Bust Adapter) - the silicon that implements the AHCI specification
// Supports up to 32 ports for attaching SATA devices such as disk drives, port multipliers, or an
// enclosure management bridge

// latest spec as of august 2024:
// https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf

#pragma once
#include "../../pci/Device.hpp"
#include "Definitions.hpp"

namespace Kernel::AHCI {

template<typename T>
class MemoryProxy {
public:
    MemoryProxy(T* _addr) : addr(_addr) {}
    operator T() const {
        return *addr;
    }
    MemoryProxy& operator=(T value) {
        *addr = value;
        return *this;
    }
    MemoryProxy& operator[](u16 n) {
        return MemoryProxy<T>{addr + n};
    }
private:
    T* addr;
};

class AHCIController : public PCI::Device {
public:
    AHCIController(const PCI::config_space_address& base);
private:
    HBARegisters* hba_regs;
};

}
