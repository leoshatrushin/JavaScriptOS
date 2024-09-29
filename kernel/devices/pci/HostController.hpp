#pragma once
#include <kernel/devices/pci/Device.hpp>
#ifdef __clang__
#include <kernel/libcppk/clang_generator.hpp>
#else
#include <generator>
#endif

namespace Kernel::PCI {


class HostController {
public:
    using device_generator = std::generator<DeviceBase>;
    device_generator attached_devices();
};

}
