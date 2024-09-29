#pragma once
#include "../libk/ErrorOr.h"

namespace Kernel::Memory {

template<typename T>
struct TypedMapping {
};

template<typename T>
static
ErrorOr<TypedMapping<T>>
map_typed(void* paddr, size_t length, Region::Access access = Region::Access::Read)
{
    TypedMapping<T> table;
    auto mapping_length = TRY(page_round_up(paddr.offset_in_page() + length));
    table.region = TRY(MM.allocate_mmio_kernel_region(paddr.page_base(), mapping_length, {}, access));
    table.offset = paddr.offset_in_page();
    table.paddr = paddr;
    table.length = length;
    return table;
}

}
