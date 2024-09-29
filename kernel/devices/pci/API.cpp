#include "API.hpp"

namespace Kernel::PCI {

BARSpaceType get_BAR_space_type(BAR_t bar) {
    if (bar.raw & (1 << 0))
        return BARSpaceType::IOSpace;

    switch (bar.memory.type) {
    case 0:
        return BARSpaceType::Memory32BitSpace;
    case 1:
        return BARSpaceType::Memory16BitSpace;
    case 2:
        return BARSpaceType::Memory64BitSpace;
    default:
        debug_printf("Invalid BAR space type\n");
        hlt();
        return BARSpaceType::IOSpace;
    }
}

}
