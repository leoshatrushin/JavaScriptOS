#include <kernel/api/posix/errno.h>
#include <kernel/libk/Assertions.h>
#include <kernel/devices/pci/Device.hpp>
#include <kernel/devices/pci/API.hpp>

using namespace Kernel::PCI;

/*template<u8 max_BAR>*/
/*template<typename T>*/
/*ErrorOr<Kernel::Memory::TypedMapping<T>>*/
/*BARMixin<max_BAR>::mapped_BAR(u8 n) {*/
/*    if (n > max_BAR) return EINVAL;*/
/*    BAR_t bar = BAR(n);*/
/*    BARSpaceType type = get_BAR_space_type(bar);*/
/*    if (type == BARSpaceType::IOSpace) return EIO;*/
/**/
/*    auto bar_address = TRY(BAR_address(n));*/
/**/
/*    return Kernel::Memory::TypedMapping<T>{bar_address};*/
/*}*/

template<u8 max_BAR>
ErrorOr<void*>
BARMixin<max_BAR>::BAR_address(u8 n) {
    BAR_t bar = BAR(n);
    BARSpaceType type = get_BAR_space_type(bar);
    if (type == BARSpaceType::IOSpace) return EIO;

    if (type == BARSpaceType::Memory64BitSpace) {
        // FIXME: In theory, the last BAR cannot be assigned to 64 bit as it is the last one...
        // however, there might be a 64 bit last BAR for real bare metal hardware, so remove this
        // if it makes a problem.
        if (n >= max_BAR) return EINVAL;

        BAR_t next_bar = BAR(n + 1);
        return ((u64)next_bar.base_addr() << 32) | bar.base_addr();
    } else {
        return bar.base_addr();
    }
}

template<u8 max_BAR>
size_t
BARMixin<max_BAR>::BAR_space_size(u8 n) {
    VERIFY(n < 6);
    command command_reg_saved = this->command_reg();
    this->command_reg() &= ~(command){ .io_space = 0, .memory_space = 0 };
    BAR_t bar_saved = BAR(n);
    BARSpaceType type = get_BAR_space_type(bar_saved);
    BAR(n) = 0xFFFFFFFF;
    size_t encoded_size = BAR(n).base_addr();
    BAR_t next_bar_saved;
    if (type == BARSpaceType::Memory64BitSpace) {
        // FIXME: see FIXME in BAR_address
        if (n >= 5) return 0;
        next_bar_saved = BAR(n + 1);
        BAR(n + 1) = 0xFFFFFFFF;
        encoded_size |= BAR(n).base_addr() << 32;
    }
    BAR(n) = bar_saved;
    if (type == BARSpaceType::Memory64BitSpace) {
        BAR(n + 1) = next_bar_saved;
    }
    this->command_reg() = command_reg_saved;
    return ~encoded_size + 1;
}
