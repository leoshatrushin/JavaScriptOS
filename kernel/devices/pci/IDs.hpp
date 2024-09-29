#pragma once
#include "../../libk/types.h"

namespace Kernel::PCI {

// https://devicehunt.com/about - stays up to date with below + extra info
// https://pci-ids.ucw.cz/ - includes mistakes found
// https://pcisig.com/membership/member-companies - official PCI-SIG member companies
enum class VendorID : u16 {
    VirtIO = 0x1af4,
    Intel = 0x8086,
    WCH = 0x1c00,
    RedHat = 0x1b36,
    Realtek = 0x10ec,
    QEMUOld = 0x1234,
    VirtualBox = 0x80ee,
    VMWare = 0x15ad,
    Tdfx = 0x121a,
    DeviceNotPresent = 0xFFFF,
};

enum class DeviceID : u16 {
    VirtIONetAdapter = 0x1000,
    VirtIOBlockDevice = 0x1001,
    VirtIOConsole = 0x1003,
    VirtIOEntropy = 0x1005,
    VirtIOGPU = 0x1050,
    VirtIOInput = 0x1052,
};

}
