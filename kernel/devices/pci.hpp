#pragma once
#include "../libk/types.h"
#include "../arch/x86/asm.h"

namespace PCI {

// possible later versions of spec?
// https://www.coursehero.com/file/69706716/PCI-Firmware-Specification-Revision-3-1-2010pdf/
// https://download.csdn.net/download/marktsai0316/10702252?ydreferer=aHR0cHM6Ly93d3cuZ29vZ2xlLmNvbS8%3D

// links, haven't checked if latest versoin
// https://picture.iczhiku.com/resource/eetop/ShiwdiOJzZErtcNx.pdf
// https://lekensteyn.nl/files/docs/PCI_SPEV_V3_0.pdf

// PCI Local Bus Specification rev 3.0
// Systems must provide a mechanism that allows software to generate PCI configuration transactions
// This mechanism is typically located in the host bridge
// This spec specifies the mechanism for PC-AT compatible systems

static constexpr u16 address_port = 0xCF8;
static constexpr u16 data_port = 0xCFC;

union config_address {
    struct {
    u8 offset; // 4-byte aligned register offset in device configuration space
    u8 function : 3; // device function, 0-8
    u8 device : 5; // bus device, 0-31
    u8 bus : 8; // PCI bus
    u8 reserved : 7;
    u8 enable : 1; // access to CONFIG_DATA register should translate to PCI bus configuration transaction
    } __attribute__((packed));
    u32 raw;
    explicit config_address(u8 bus, u8 device, u8 function) :
        bus(bus), device(device), function(function), enable(1) {}
    u32 operator+(u8 offset) const {
        config_address offset_addr {*this};
        offset_addr.offset = offset & 0xFC;
        return offset_addr.raw;
    }
};

// https://devicehunt.com/about - stays up to date with below + extra info
// https://pci-ids.ucw.cz/ - includes mistakes found
// https://pcisig.com/membership/member-companies - official PCI-SIG member companies
enum pci_vendor_id {
    PCI_VENDOR_ID_INVALID = 0xFFFF,
};

union command_reg {
    struct {
    u8 io_space : 1; // I/O space accesses enabled...
    u8 memory_space : 1; // memory space accesses enabled, otherwise configuration space accesses only
    u8 bus_master : 1; // can behave as a bus master, otherwise cannot generate PCI accesses
    u8 special_cycles : 1; // can monitor special cycle operations, otherwise ignores them
    u8 memory_write_and_invalidate : 1; // RO, can generate memory write and invalidate command
                                        // otherwise memory write command must be used
    u8 vga_palette_snoop : 1; // RO, do not respond to pallete register writes and snoop the data
                              // otherwise trace palette write accesses like all other accesses
    u8 parity_error_response : 1; // 0 = set bit 15, do not assert PERR# pin, continue, 1 = normal action
    u8 rsrv0 : 1; // RO
    u8 serr : 1; // enable SERR# driver
    u8 fast_back_to_back : 1; // RO, allowed to generate fast back-to-back transactions outside the same agent
    u8 interrupt_disable : 1; // disable assertion of IOAPIC interrupt INTx# signals (instead use MSI/MSI-X)
    u8 rsrv1 : 5;
    } __attribute__((packed));
    u16 raw;
    command_reg(u16 raw) : raw(raw) {}
    operator u16() const { return raw; }
};

union status_reg {
    struct {
        u8 rsrv0 : 3;
        u8 interrupt_status : 1; // state of device's INTx# signal, asserted if interrupts enabled
        u8 capabilities_list : 1; // implements pointer for new capabilities list at offset 0x34
        u8 _66mhz_capable : 1; // otherwise 33 MHz
        u8 rsrv1 : 1;
        u8 fast_back_to_back_capable : 1; // can accept fast back-to-back transactions not from the same agent
        u8 master_data_parity_error : 1; // bus agent asserted PERR# on read or observed assertion on write
                                         // + agent setting the bit acted as the bus master for the operation
                                         // + parity error response bit = 1
        u8 devsel_timing : 2; // RO, slowest time a device will assert DDEVSEL# for any bus command except
                              // configuration space reads and writes
                              // 0 = fast timing, 1 = medium, 2 = slow
        u8 signaled_target_abort : 1; // target device terminates transaction with target-abort
        u8 received_target_abort : 1; // set by a master device when its transaction terminated w target-abort
        u8 received_master_abort : 1; // set by a master device when its transaction terminate w master-abort
                                      // except special cycle transactions
        u8 signaled_system_error : 1; // device asserts SERR#
        u8 detected_parity_error : 1; // detected parity error, even if parity error handling disabled
    } __attribute__((packed));
    u16 raw;
    status_reg(u16 raw) : raw(raw) {}
    operator u16() const { return raw; }
};

// not latest version of the spec
// https://pcisig.com/sites/default/files/files/PCI_Code-ID_r_1_11__v24_Jan_2019.pdf
// https://pci-ids.ucw.cz/
enum class class_code_reg {
    Legacy = 0x00,
    MassStorageController = 0x01,
    NetworkController = 0x02,
    DisplayController = 0x03,
    MultimediaController = 0x04,
    MemoryController = 0x05,
    BridgeDevice = 0x06,
    SimpleCommunicationController = 0x07,
    BaseSystemPeripheral = 0x08,
    InputDevice = 0x09,
    DockingStation = 0x0A,
    Processor = 0x0B,
    SerialBusController = 0x0C,
    WirelessController = 0x0D,
    IntelligentIOController = 0x0E,
    SatelliteCommunicationController = 0x0F,
    EncryptionController = 0x10,
    DataAcquisitionAndSignalProcessingController = 0x11,
    ProcessingAccelerator = 0x12,
    NonEssentialInstrumentation = 0x13,
    Other = 0xFF
};

enum class __attribute__((packed)) header_type {
    GENERAL_DEVICE = 0x00,
    PCI_TO_PCI_BRIDGE = 0x01,
    PCI_TO_CARDBUS_BRIDGE = 0x02,
};

union header_type_reg {
    struct {
    enum header_type type : 7;
    u8 is_multi_function : 1;
    } __attribute__((packed));
    u8 raw;
    header_type_reg(u8 raw) : raw(raw) {}
    operator u8() const { return raw; }
};

// PCI is always little-endian
// common header fields - supported by all PCI compliant devices (except listed exceptions)
#define PCI_CONFIG_REGISTERS(X) \
    X(u16, vendor_id,       0x00) /* pcisig.com/membership/member-companies, 0xFFFF = device not present */ \
    X(u16, device_id,       0x02)  \
    X(command_reg, command,         0x04)  \
    X(status_reg, status,          0x06)  \
    X(u8,  revision_id,     0x08)  \
    X(u8,  prog_if,         0x09)  \
    X(u8,  subclass,        0x0A)  \
    X(class_code_reg,  class_code,      0x0B)  \
    X(u8,  cache_line_size, 0x0C) /* 32-bit units, subject to device support, invalid behaves as 0 */ \
    X(u8,  latency_timer,   0x0D) /* optional, PCI bus block units */ \
    X(header_type_reg,  header_type,     0x0E)  \
    X(u8,  BIST,            0x0F) /* optional */ \
    X(u8,  interrupt_line,   0x3C) /* PIC interrupt */ \
    X(u8,  interrupt_pin,    0x3D) /* I/O APIC interrupt, INTA#, INTB#, INTC#, or INTD# */

#define BAR_REG_IS_IO_SPACE(bar) ((bar & 1) == 1)

enum class memory_space_bar_type {
    _32_BIT = 0x0,
    _64_BIT = 0x2, // takes up 2 BAR slots
};

union memory_space_bar {
    struct {
    u8 always1 : 1;
    enum memory_space_bar_type type : 2;
    u8 prefetchable: 1; // w/ paging, for maximum performance, map region as WT instead of UC (uncacheable)
                        // on x86, frame buffers are the exception, they shoudl almost always be mapped as WC
    u32 base_addr : 1; // 16-byte aligned
    } __attribute__((packed));
    u32 raw;
};

union io_space_bar {
    struct {
    u8 always0 : 1;
    u8 rsrv0 : 1;
    u32 base_addr : 1; // 4-byte aligne
    } __attribute__((packed));
    u16 raw;
};

#define PCI_HEADER_TYPE0_CONFIG_REGISTERS(X) \
    X(u32, BAR0,             0x10)  /* Base Address Registers of additional configuration space */ \
    X(u32, BAR1,             0x14)  \
    X(u32, BAR2,             0x18)  \
    X(u32, BAR3,             0x1C)  \
    X(u32, BAR4,             0x20)  \
    X(u32, BAR5,             0x24)  \
    X(u32, cardbus_CIS_ptr,  0x28)  \
    X(u16, subsystem_vendor_id, 0x2C) \
    X(u16, subsystem_id,     0x2E)  \
    X(u32, expansion_ROM_base_addr, 0x30) \
    X(u8,  capabilities_list_offset, 0x34)  \
    X(u8,  min_grant,        0x3E)  \
    X(u8,  max_latency,      0x3F)

#define PCI_HEADER_TYPE1_CONFIG_REGISTERS(X) \
    X(u32, BAR0,             0x10)  \
    X(u32, BAR1,             0x14)  \
    X(u8,  primary_bus,      0x18)  \
    X(u8,  secondary_bus,    0x19)  \
    X(u8,  subordinate_bus,  0x1A)  \
    X(u8,  secondary_latency_timer, 0x1B) \
    X(u8,  io_base_lower,          0x1C)  \
    X(u8,  io_limit_lower,         0x1D)  \
    X(u16, secondary_status, 0x1E)  \
    X(u16, memory_base,      0x20)  \
    X(u16, memory_limit,     0x22)  \
    X(u16, prefetchable_memory_base, 0x24) \
    X(u16, prefetchable_memory_limit, 0x26) \
    X(u32, prefetchable_base_upper, 0x28) \
    X(u32, prefetchable_limit_upper, 0x2C) \
    X(u16, io_base_upper,    0x30)  \
    X(u16, io_limit_upper,   0x32)  \
    X(u8, capabilities_list_offset, 0x34) \
    X(u32, expansion_ROM_base_addr, 0x38) \
    X(u16, bridge_control,   0x3E)

#define PCI_HEADER_TYPE2_CONFIG_REGISTERS(X) \
    X(u32, cardbus_socket_slash_exca_base_addr, 0x10)  \
    X(u8,  capabilities_list_offset, 0x14)  \
    X(u16, secondary_status,  0x16)  \
    X(u8,  pci_bus_num,       0x18)  \
    X(u8,  cardbus_bus_num,   0x19)  \
    X(u8,  subordinate_bus_num, 0x1A)  \
    X(u8,  cardbus_latency_timer,     0x1B)  \
    X(u32, memory_base0,      0x1C)  \
    X(u32, memory_limit0,     0x20)  \
    X(u32, memory_base1,      0x24)  \
    X(u32, memory_limit1,     0x28)  \
    X(u32, io_base0,           0x2C)  \
    X(u32, io_limit0,          0x30)  \
    X(u32, io_base1,           0x34)  \
    X(u32, io_limit1,          0x38)  \
    X(u16, bridge_control,     0x3E) \
    X(u16, subsystem_device_id, 0x40) \
    X(u16, subsystem_vendor_id, 0x42) \
    X(u32, _16_bit_pc_card_legacy_mode_base_addr,   0x44)

template <typename T>
class RegisterProxy {
public:
    RegisterProxy(u8 offset, config_address base) : offset(offset), base(base) {}

    operator T() {
        return read_register(offset);
    }

    RegisterProxy& operator=(T value) {
        write_register(offset, value);
        return *this;
    }

private:
    // https://wiki.osdev.org/PCI
    // "all reads and writes must be both 32-bits and aligned to work on all implementations"
    u32 read_register(u8 offset) {
        out32(PCI::address_port, base + offset);
        return in32(PCI::data_port) >> ((offset & 0b11) * 8);
    }

    void write_register8(u8 offset, u8 value) {
        out32(PCI::address_port, base + offset);
        u32 cur = in32(PCI::data_port);
        cur &= ~(0xFF << ((offset & 0b11) * 8));
        cur |= value << ((offset & 0b11) * 8);
        out32(PCI::data_port, cur);
    }

    void write_register16(u8 offset, u16 value) {
        out32(PCI::address_port, base + offset);
        u32 cur = in32(PCI::data_port);
        cur &= ~(0xFFFF << ((offset & 0b11) * 8));
        cur |= value << ((offset & 0b11) * 8);
        out32(PCI::data_port, cur);
    }

    void write_register32(u8 offset, u32 value) {
        out32(PCI::address_port, base + offset);
        out32(PCI::data_port, value);
    }

    void write_register(u8 offset, T value) {
        if constexpr (sizeof(T) == 1) {
            write_register8(offset, static_cast<u8>(value));
        } else if constexpr (sizeof(T) == 2) {
            write_register16(offset, static_cast<u16>(value));
        } else if constexpr (sizeof(T) == 4) {
            write_register32(offset, static_cast<u32>(value));
        } else {
            static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Unsupported register size");
        }
    }

private:
    u8 offset;
    config_address base;
};

#define CREATE_ACCESSORS(type, name, offset) \
    RegisterProxy<type> name() { return RegisterProxy<type>{offset, base}; }

class PCIDeviceConfigSpaceProxyBase {
public:
    PCIDeviceConfigSpaceProxyBase(const config_address& base) : base(base) {}
    PCI_CONFIG_REGISTERS(CREATE_ACCESSORS)
    enum header_type proxy_type;
protected:
    config_address base;
};

class PCIDeviceConfigSpaceProxyType0 : public PCIDeviceConfigSpaceProxyBase {
public:
    PCIDeviceConfigSpaceProxyType0(const config_address& base) : PCIDeviceConfigSpaceProxyBase(base) {
        proxy_type = header_type::GENERAL_DEVICE;
    }
    PCI_HEADER_TYPE0_CONFIG_REGISTERS(CREATE_ACCESSORS)
};

class PCIDeviceConfigSpaceProxyType1 : public PCIDeviceConfigSpaceProxyBase {
public:
    PCIDeviceConfigSpaceProxyType1(const config_address& base) : PCIDeviceConfigSpaceProxyBase(base) {
        proxy_type = header_type::PCI_TO_PCI_BRIDGE;
    }
    PCI_HEADER_TYPE1_CONFIG_REGISTERS(CREATE_ACCESSORS)
};

class PCIDeviceConfigSpaceProxyType2 : public PCIDeviceConfigSpaceProxyBase {
public:
    PCIDeviceConfigSpaceProxyType2(const config_address& base) : PCIDeviceConfigSpaceProxyBase(base) {
        proxy_type = header_type::PCI_TO_CARDBUS_BRIDGE;
    }
    PCI_HEADER_TYPE2_CONFIG_REGISTERS(CREATE_ACCESSORS)
};

class PCIDeviceConfigSpace {
public:
    static PCIDeviceConfigSpaceProxyBase create(u8 bus, u8 device, u8 function) {
        config_address base{bus, device, function};
        PCIDeviceConfigSpaceProxyBase base_proxy(base);
        header_type_reg proxy_type = base_proxy.header_type();
        switch (proxy_type.type) {
            case header_type::GENERAL_DEVICE:
                return PCIDeviceConfigSpaceProxyType0(base);
            case header_type::PCI_TO_PCI_BRIDGE:
                return PCIDeviceConfigSpaceProxyType1(base);
            case header_type::PCI_TO_CARDBUS_BRIDGE:
                return PCIDeviceConfigSpaceProxyType2(base);
        }
    }
};

enum __attribute__((packed)) pci_capability {
    MSI = 0x05,
    MSIX = 0x11,
};

typedef union {
    struct {
    enum pci_capability capability_id;
    u8 next_capability_pointer;
    };
    u16 raw;
} __attribute__((packed)) pci_capaibility_common;

enum capability_common_reg_offset {
    CAPABILITY_ID_REG_OFFSET = 0x00,
    NEXT_CAPABILITY_POINTER_REG_OFFSET = 0x01,
};

enum msix_capability_reg_offset {
    MSIX_MESSAGE_CONTROL_REG_OFFSET = 0x02,
    MSIX_BIR_OFFSET = 0x04, // BAR indicator register
    MSIX_TABLE_OFFSET_REG_OFFSET = 0x05,
    MSIX_PENDING_BIT_BIR_REG_OFFSET = 0x08,
    MSIX_PENDING_BIT_OFFSET_REG_OFFSET = 0x09,
};

typedef union {
    struct {
    u16 table_size : 11; // RO, n-1 MSIX table entries
    u8 rsrv0 : 3;
    u8 function_mask : 1;
    u8 enable : 1;
    };
    u16 raw;
} __attribute__((packed)) msix_message_control_reg;

typedef union {
    struct {
    u64 message_address; // 4-byte aligned
    u32 message_data;
    struct {
        u8 masked : 1;
        u32 rsrv0 : 31;
    } vector_control;
    u32 table_offset : 12; // 4-byte aligned
    u8 rsrv0 : 3;
    u8 bir : 3; // BAR indicator register
    u8 offset : 1; // 0 = 32-bit, 1 = 64-bit
    };
    u16 raw;
} __attribute__((packed)) msix_table_entry;

enum LegacySubclassID {
    Any = 0x00,
    VGACompatible = 0x01
};

enum MassStorageSubclassID {
    SCSIBusController = 0x00,
    IDEController = 0x01,
    FloppyDiskController = 0x02,
    IPIBusController = 0x03,
    RAIDController = 0x04,
    ATAController = 0x05,
    SATAController = 0x06,
    SASController = 0x06,
    NVMeController = 0x08 // non-volatile memory
};

enum SATAProgIF {
    VendorSpecific = 0x00,
    AHCI = 0x01,
    SerialStorageBus = 0x02
};

enum NetworkSubclassID { // controllers
    Ethernet = 0x00,
    TokenRing = 0x01,
    FDD = 0x02,
    ATM = 0x03,
    ISDN = 0x04,
    WorldFlip = 0x05,
    PICMG_2_14_MultiComputing = 0x06,
    InfiniBand = 0x07,
    HostFabric = 0x08,
};

enum DisplaySubclassID {
    VGA = 0x00,
    XGA = 0x01,
    ThreeD = 0x02,
};

enum VGAProgIF {
    VGACompatibleController = 0x00, // mem 0xA0000-0xBFFFF, I/O 0x3B0-0x3BB, 0x3C0-0x3DF + aliases
    _8514Compatible = 0x01, // I/O addresses 0x2E8 and aliases 0x2Ea-0x2EF
};

enum MultimediaSubclassID {
    VideoDevice = 0x00,
    AudioDevice = 0x01,
    ComputerTelephonyDevice = 0x01,
    HDACompatibleAudioDevice = 0x3,
};

enum HDACompatibleAudioProgIF {
    Compatible = 0x00,
    CompatibleWithVendorExtensions = 0x80,
};

enum MemorySubclassID {
    RAM = 0x00,
    Flash = 0x01,
};

enum BridgeSubclassID {
    Host = 0x00,
    ISA = 0x01,
    PCI_TO_PCI = 0x4,
    CardBus = 0x07,
    PCI_TO_PCI_SemiTransparent = 0x09,
};

enum PCI_TO_PCIProgIF {
    Standard = 0x00,
    SupportsSubtractiveDecode = 0x01,
};

enum SimpleCommunicationSubclassID {
    SerialController = 0x00,
};

enum SerialControllerProgIF {
    _8250CompatibleGenericXT = 0x00,
    _16450Compatible = 0x00,
    _16550Compatible = 0x01,
    _16650Compatible = 0x02,
    _16750Compatible = 0x03,
    _16850Compatible = 0x04,
    _16950Compatible = 0x05,
};

enum SubclassID {
    PIC = 0x00,
    DMAController = 0x01,
    Timer = 0x02,
    RTCController = 0x03,
    PCIHotplugController = 0x04,
    SDHostController = 0x5,
    IOMMU = 0x06
};

enum PICProgIF {
    Generic8259Compatible = 0x00,
    ISACompatible = 0x01,
    EISACompatible = 0x02,
    IOAPICInterruptController = 0x10,
    IOxAPICInterruptController = 0x20,
};

enum DMAControllerProgIF {
    Generic8237Compatible = 0x00,
    ISADMAController = 0x01,
    EISADMAController = 0x02,
};

enum TimerProgIF {
    Generic8254Timer = 0x00,
    ISATimer = 0x01,
    EISATimers = 0x02,
    HPET = 0x03,
};

enum InputDeviceSubclassID {
    KeyboardController = 0x00,
    DigitizerPen = 0x01,
    MouseController = 0x02,
    ScannerController = 0x03,
    GameportController = 0x04,
};

enum SerialBusSubclassID { // controllers
    FireWire = 0x00,
    USB = 0x03,
    SMBus = 0x05,
};

enum USBProgIF {
    UHCI = 0x00,
    OHCI = 0x01,
    EHCI = 0x02, // USB2
    XHCI = 0x03, // USB3
    Unspecified = 0x80,
    USBDevice = 0xFE, // (not a host controller)
};

enum WirelessSubclassID {
    BluetoothController = 0x11,
    Ethernet802_1aController = 0x20, // 5 GHz
    Ethernet802_1bController = 0x21, // 2.4 GHz
};

}

void enumerate_all_pci_buses();
