#pragma once
#include "Definitions.hpp"
#include "Device.hpp"

namespace Kernel::PCI {

BARSpaceType get_BAR_space_type(BAR_t bar);

}
