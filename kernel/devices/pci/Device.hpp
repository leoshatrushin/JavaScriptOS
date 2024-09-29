#pragma once
#include <kernel/libk/types.h>
#include <kernel/libk/ErrorOr.hpp>
#include <kernel/console.h>
#include <kernel/arch/x86/asm.hpp>
#include <kernel/libcppk/optional.hpp>
#include <kernel/devices/pci/Definitions.hpp>
#include <kernel/devices/pci/IDs.hpp>

namespace Kernel::PCI {

// possible later versions of spec?
// https://www.coursehero.com/file/69706716/PCI-Firmware-Specification-Revision-3-1-2010pdf/
// https://download.csdn.net/download/marktsai0316/10702252?ydreferer=aHR0cHM6Ly93d3cuZ29vZ2xlLmNvbS8%3D

// links, haven't checked if latest versoin
// https://picture.iczhiku.com/resource/eetop/ShiwdiOJzZErtcNx.pdf
// https://lekensteyn.nl/files/docs/PCI_SPEV_V3_0.pdf

// PCI Local Bus Specification rev 3.0

static constexpr u16 address_port = 0xCF8;
static constexpr u16 data_port = 0xCFC;

typedef u8 config_space_offset;

union config_space_address {
    struct {
    config_space_offset offset; // 4-byte aligned register offset in device configuration space
    u8 function : 3; // device function, 0-8
    u8 device : 5; // bus device, 0-31
    u8 bus : 8; // PCI bus
    u8 reserved : 7;
    u8 enable : 1; // access to CONFIG_DATA register should translate to PCI bus configuration transaction
    } __attribute__((packed));
    u32 raw;
    //operator u32() const { return raw; } // TODO: this makes base + offset (int) ambiguous
    explicit config_space_address(u8 bus, u8 device, u8 function) :
        offset{0}, function{function}, device{device}, bus{bus}, reserved{0}, enable{1} {}
    explicit config_space_address(u32 raw) : raw{raw} {}
    config_space_address base() const {
        return config_space_address {bus, device, function};
    }
    void set_offset(u8 offset) { // TODO: maybe make offset private somehow
        this->offset = offset & ~0b11;
    }
    config_space_address operator+(u8 offset) const {
        config_space_address result {*this};
        result.offset += offset & ~0b11;
        return result;
    }
    config_space_address operator+=(u8 offset) {
        offset = offset & 0xFC;
        return *this + offset;
    }
};

union command {
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
    operator u16() const { return raw; }
};

union status {
    struct {
        u8 rsrv0 : 3;
        u8 interrupt_status : 1; // state of device's INTx# signal, asserted if interrupts enabled
        u8 has_capabilities_list : 1; // implements pointer for new capabilities list at offset 0x34
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
    status(u16 raw) : raw(raw) {}
    operator u16() const { return raw; }
};

template <typename T>
class RegisterProxy {
public:
    RegisterProxy(config_space_address _base, u8 _offset) : base{_base}, offset{_offset} {

    }

    class MemberProxy {
    public:
        MemberProxy(RegisterProxy& _parent, T _value) : parent(_parent), value(_value) {}
        T* operator->() { return &value; }
        ~MemberProxy() { parent.write_register(value); }
    private:
        RegisterProxy& parent;
        T value;
    };

    MemberProxy operator->() {
        T value = read_register();
        return MemberProxy{*this, value};
    }

    operator T() {
        return static_cast<T>(read_register());
    }

    // when you need a manual cast
    T cast() {
        return static_cast<T>(read_register());
    }

    RegisterProxy& operator=(T value) {
        write_register(value);
        return *this;
    }

    RegisterProxy& operator|=(u32 value) {
        write_register(read_register() | value);
        return *this;
    }

    RegisterProxy& operator&=(u32 value) {
        write_register(read_register() & value);
        return *this;
    }

private:
    // https://wiki.osdev.org/PCI
    // "all reads and writes must be both 32-bits and aligned to work on all implementations"
    u32 read_register() {
        out32(PCI::address_port, (base + offset).raw);
        return in32(PCI::data_port) >> ((offset & 0b11) * 8);
    }

    void write_register8(u8 value) {
        out32(PCI::address_port, (base + offset).raw);
        u32 cur = in32(PCI::data_port);
        cur &= ~(0xFF << ((offset & 0b11) * 8));
        cur |= value << ((offset & 0b11) * 8);
        out32(PCI::data_port, cur);
    }

    void write_register16(u16 value) {
        out32(PCI::address_port, (base + offset).raw);
        u32 cur = in32(PCI::data_port);
        cur &= ~(0xFFFF << ((offset & 0b11) * 8));
        cur |= value << ((offset & 0b11) * 8);
        out32(PCI::data_port, cur);
    }

    void write_register32(u32 value) {
        out32(PCI::address_port, (base + offset).raw);
        out32(PCI::data_port, value);
    }

    void write_register(T value) {
        if constexpr (sizeof(T) == 1) {
            write_register8(value);
        } else if constexpr (sizeof(T) == 2) {
            write_register16(value);
        } else if constexpr (sizeof(T) == 4) {
            write_register32(value);
        } else {
            static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Unsupported register size");
        }
    }

private:
    config_space_address base;
    u8 offset;
    mystd::optional<T> value;
};

enum class __attribute__((packed)) header_type {
    GENERAL_DEVICE = 0x00,
    PCI_TO_PCI_BRIDGE = 0x01,
    PCI_TO_CARDBUS_BRIDGE = 0x02,
};

union device_header_type {
    struct {
    enum header_type type : 7;
    u8 is_multi_function : 1;
    } __attribute__((packed));
    u8 raw;
    device_header_type(u8 raw) : raw(raw) {}
    operator u8() const { return raw; }
};

static constexpr u8 BAR0_offset = 0x10;

union memory_space_bar {
    struct {
    u8 always1 : 1;
    u8 type : 2;
    u8 prefetchable: 1; // No read side effects, thus can cache data and read in bursts
                        // Also host bridges can merge writes into just the latest write
                        // w/ paging, for maximum performance, map region as WT instead of UC (uncacheable)
                        // on x86, frame buffers are the exception, they should almost always be mapped as WC
    u32 base_addr_mem : 28; // 16-byte aligned
    } __attribute__((packed));
    u32 raw;
    u32 base_addr() const { return raw & ~0xF; }
};

union io_space_bar {
    struct {
    u8 always0 : 1;
    u8 rsrv0 : 1;
    u32 base_addr : 30; // 4-byte aligneg
    } __attribute__((packed));
    u32 raw;
};

union BAR_t {
    memory_space_bar memory;
    io_space_bar io;
    u32 raw;
    u32 base_addr() const { return (raw & 1) ? raw & ~0b1111 : raw & ~0b11; }
}; // Base Address Register

enum class device_capability : u8 {
    //MSI = 0x05,
    MSIX = 0x11,
};

union msix_message_control{
    struct {
    u16 table_size : 11; // RO, n-1 MSIX table entries
    u8 rsrv0 : 3;
    u8 function_mask : 1; // mask all vectors for this device function, overrides per-vector masking
    u8 enable : 1;
    } __attribute__((packed));
    u16 raw;
};

union msix_table_entry {
    struct {
    u32 message_addr; // must be 4-byte aligned (low bits 00)
    u32 message_upper_addr;
    u32 message_data;
    struct {
        u8 mask : 1;
        u32 rsrv0 : 31;
    } vector_control;
    } __attribute__((packed));
    u16 raw;
};

union x86_64_msi_message_address_reg {
    struct {
    u8 rsrv0z : 2;
    u8 destination_mode : 1; // 0 = physical (LAPIC ID), 1 = logical (processor set)
    u8 redirection_hint : 1; // more control for logical interrupts
    u8 rsrv1;
    u8 destination_id;
    u16 _0x0fee : 12;
    } __attribute__((packed));
    u32 raw;
    operator u32() const { return raw; }
};

// only operate in conjunction with specified trigger modes
enum class msi_delivery_mode {
    FIXED = 0, // deliver to all destination agents
    LOWEST_PRIORITY = 1, // deliver to the destination agent executing at lowest priority
    SMI = 2, // System Management Interrupt, edge-triggered only, vector must be 0
    NMI = 4,
    INIT = 5,
    EXTINT = 7,
};

enum class msi_level_trigger_mode_level {
    DEASSERT = 0,
    ASSERT = 1,
};

enum class msi_trigger_mode {
    EDGE = 0,
    LEVEL = 1,
};

union x86_64_msi_message_data_reg {
    struct {
    u8 vector; // 0x10-0xFE
    enum msi_delivery_mode delivery_mode : 3;
    u8 rsrv0 : 3;
    enum msi_level_trigger_mode_level level : 1;
    enum msi_trigger_mode trigger_mode : 1;
    u16 rsrv1;
    } __attribute__((packed));
    u32 raw;
    operator u32() const { return raw; }
};

#define ACCESSOR(type, name, offset) \
    RegisterProxy<type> name() { return RegisterProxy<type>{base, offset}; }

class Capability {
public:
    Capability(const config_space_address& _base) : base{_base} {}
    // TODO: cache capability_id during factory
    ACCESSOR(enum device_capability, capability_id, 0x00)
    ACCESSOR(config_space_offset, next_capability_pointer, 0x01)
protected:
    config_space_address base;
};

class MSIXCapability : public Capability {
public:
    MSIXCapability(const config_space_address& _base) : Capability{_base} {}
    ACCESSOR(msix_message_control, message_control, 0x02)
    u8 table_BIR() { return RegisterProxy<u8>{base, 0x04} & 0b111; } // BAR Indicator Register
    u32 table_offset() { return RegisterProxy<u32>{base, 0x04} & !0b111; }
    u8 pending_bit_array_BIR() { return RegisterProxy<u8>{base, 0x08} & 0b111; } // BAR Indicator Register
    u32 pending_bit_array_offset() { return RegisterProxy<u32>{base, 0x08} & !0b111; }
};

class CapabilityFactory {
public:
    static Capability create(config_space_address capability_base) {
        Capability base_config_space{capability_base};
        auto id = base_config_space.capability_id();
        switch (id) {
            case device_capability::MSIX:
                return MSIXCapability{capability_base};
        }
    }
};

class Capabilities {
private:
    class CapabilityIterator {
    public:
        CapabilityIterator(const config_space_address& _capability_base)
            : capability_base(_capability_base),
              capability{Capability{capability_base}} {}
        Capability operator*() { return capability; }
        CapabilityIterator& operator++() {
            config_space_offset next_capability_offset = capability.next_capability_pointer();
            if (next_capability_offset == 0) {
                capability_base.raw = 0;
            } else {
                capability_base.set_offset(next_capability_offset);
            }
            capability = CapabilityFactory::create(capability_base);
            return *this;
        }
        bool operator==(const CapabilityIterator& other) {
            return capability_base.raw == other.capability_base.raw;
        }
        bool operator!=(const CapabilityIterator& other) {
            return !(*this == other);
        }
    private:
        config_space_address capability_base;
        Capability capability;
    };
public:
    Capabilities(const config_space_address& _first_capability_base)
        : first_capability_base(_first_capability_base) {}
    CapabilityIterator begin() { return CapabilityIterator{first_capability_base}; }
    CapabilityIterator end() { return CapabilityIterator{config_space_address{0}}; }
private:
    config_space_address first_capability_base;
};

// PCI is always little-endian
// common header fields - supported by all PCI compliant devices (except listed exceptions)
class DeviceBase {
public:
    DeviceBase(u8 bus, u8 device, u8 function) : base{bus, device, function} {}
    DeviceBase(const config_space_address& _base) : base{_base} {}
    ACCESSOR(enum VendorID, vendor_id, 0x00) // vendor that built the silicon
    ACCESSOR(enum DeviceID, device_id, 0x02)  
    ACCESSOR(command, command_reg, 0x04)  
    ACCESSOR(status, status_reg, 0x06)  
    ACCESSOR(u8,  revision_id, 0x08)  
    ACCESSOR(u8,  prog_if, 0x09) // programming interface version
    ACCESSOR(u8,  subclass_id, 0x0A)  
    ACCESSOR(enum DeviceClass, class_id, 0x0B)  
    ACCESSOR(u8,  cache_line_size, 0x0C) // 32-bit units, subject to device support, invalid behaves as 0
    ACCESSOR(u8,  latency_timer, 0x0D) // optional, PCI bus block units
    ACCESSOR(device_header_type, header_type, 0x0E)  
    ACCESSOR(u8,  BIST, 0x0F) // optional
    ACCESSOR(u8,  interrupt_line, 0x3C) // PIC interrupt
    ACCESSOR(u8,  interrupt_pin, 0x3D) // I/O APIC interrupt, INTA#, INTB#, INTC#, or INTD#
    virtual Capabilities capabilities() {
        return Capabilities{config_space_address{0}};
    }
protected:
    config_space_address base;
};

// Base Address Registers of additional configuration space
// A typical device requires one BAR for control
// Some graphics devices may use one for control and another for a frame buffer
// TODO: Make this an actual Mixin that requires having inherited from DeviceBase, not just in the hierarchy
template<u8 max_BAR>
class BARMixin : public DeviceBase {
public:
    BARMixin(u8 bus, u8 device, u8 function) : DeviceBase{bus, device, function} {}
    BARMixin(const config_space_address& _base) : DeviceBase{_base} {}
    RegisterProxy<BAR_t> BAR(u8 n) {
        if (n > max_BAR) {
            debug_printf("Invalid BAR index\n");
            hlt();
        }
        return RegisterProxy<BAR_t>{this->base + (u8)(BAR0_offset + 4 * n)};
    }
    ErrorOr<void*> BAR_address(u8 n);
    size_t BAR_space_size(u8 n);
};

class Device : public BARMixin<5> {
public:
    Device(u8 bus, u8 device, u8 function) : BARMixin{bus, device, function} {}
    Device(const config_space_address& base) : BARMixin{base} {}
    ACCESSOR(u32, cardbus_CIS_ptr,  0x28)
    ACCESSOR(VendorID, subsystem_vendor_id, 0x2C) // vendor that built the add-in card
    ACCESSOR(u16, subsystem_id,     0x2E)
    ACCESSOR(u32, expansion_ROM_base_addr, 0x30)
    ACCESSOR(u8,  min_grant,        0x3E)
    ACCESSOR(u8,  max_latency,      0x3F)
    Capabilities capabilities() {
        return Capabilities{base + 0x34};
    }
};

class PCIToPCIBridge : public BARMixin<2> {
public:
    PCIToPCIBridge(u8 bus, u8 device, u8 function) : BARMixin{bus, device, function} {}
    PCIToPCIBridge(const config_space_address& base) : BARMixin{base} {}
    ACCESSOR(u8,  primary_bus,      0x18)
    ACCESSOR(u8,  secondary_bus,    0x19)
    ACCESSOR(u8,  subordinate_bus,  0x1A)
    ACCESSOR(u8,  secondary_latency_timer, 0x1B)
    ACCESSOR(u8,  io_base_lower,          0x1C) 
    ACCESSOR(u8,  io_limit_lower,         0x1D) 
    ACCESSOR(u16, secondary_status, 0x1E)
    ACCESSOR(u16, memory_base,      0x20)
    ACCESSOR(u16, memory_limit,     0x22)
    ACCESSOR(u16, prefetchable_memory_base, 0x24)
    ACCESSOR(u16, prefetchable_memory_limit, 0x26)
    ACCESSOR(u32, prefetchable_base_upper, 0x28)
    ACCESSOR(u32, prefetchable_limit_upper, 0x2C)
    ACCESSOR(u16, io_base_upper,    0x30)
    ACCESSOR(u16, io_limit_upper,   0x32)
    ACCESSOR(u32, expansion_ROM_base_addr, 0x38)
    ACCESSOR(u16, bridge_control,   0x3E)
    Capabilities capabilities() {
        return Capabilities{base + 0x34};
    }
};

class PCIToCardBusBridge : public DeviceBase {
public:
    PCIToCardBusBridge(u8 bus, u8 device, u8 function) : DeviceBase{bus, device, function} {}
    PCIToCardBusBridge(const config_space_address& base) : DeviceBase{base} {}
    ACCESSOR(u32, cardbus_socket_slash_exca_base_addr, 0x10)
    ACCESSOR(u16, secondary_status,  0x16)
    ACCESSOR(u8,  pci_bus_num,       0x18)
    ACCESSOR(u8,  cardbus_bus_num,   0x19)
    ACCESSOR(u8,  subordinate_bus_num, 0x1A)
    ACCESSOR(u8,  cardbus_latency_timer,     0x1B)
    ACCESSOR(u32, memory_base0,      0x1C)
    ACCESSOR(u32, memory_limit0,     0x20)
    ACCESSOR(u32, memory_base1,      0x24)
    ACCESSOR(u32, memory_limit1,     0x28)
    ACCESSOR(u32, io_base0,           0x2C) 
    ACCESSOR(u32, io_limit0,          0x30) 
    ACCESSOR(u32, io_base1,           0x34) 
    ACCESSOR(u32, io_limit1,          0x38) 
    ACCESSOR(u16, bridge_control,     0x3E)
    ACCESSOR(u16, subsystem_device_id, 0x40)
    ACCESSOR(u16, subsystem_vendor_id, 0x42)
    ACCESSOR(u32, _16_bit_pc_card_legacy_mode_base_addr,   0x44)
    Capabilities capabilities() {
        return Capabilities{base + 0x14};
    }
};

class DeviceFactory {
public:
    static DeviceBase create(u8 bus, u8 device, u8 function) {
        config_space_address base{bus, device, function};
        DeviceBase device_base{base};
        device_header_type proxy_type = device_base.header_type();
        // TODO: AHCIController, check vendor id and device id to really be sure
        switch (proxy_type.type) {
            case header_type::GENERAL_DEVICE:
                return Device{base};
            case header_type::PCI_TO_PCI_BRIDGE:
                return PCIToPCIBridge{base};
            case header_type::PCI_TO_CARDBUS_BRIDGE:
                return PCIToCardBusBridge{base};
        }
        return device_base;
    }
};


}
