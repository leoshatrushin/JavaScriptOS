#pragma once
#include "../../libk/types.h"

namespace Kernel::PCI {

enum class BARSpaceType {
    IOSpace,
    Memory16BitSpace,
    Memory32BitSpace,
    Memory64BitSpace, // takes up 2 BAR slots
};

// not latest version of the spec
// https://pcisig.com/sites/default/files/files/PCI_Code-ID_r_1_11__v24_Jan_2019.pdf
// https://pci-ids.ucw.cz/
enum class DeviceClass : u8 {
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

namespace Legacy {

enum class SubclassID : u8 {
    Any = 0x00,
    VGACompatible = 0x01
};

}

namespace MassStorage {

enum class SubclassID : u8 { // controllers
    SCSIBus = 0x00,
    IDE = 0x01,
    FloppyDisk = 0x02,
    IPIBus = 0x03,
    RAID = 0x04,
    ATA = 0x05,
    SATA = 0x06,
    SAS = 0x06,
    NVMe = 0x08 // non-volatile memory
};

enum class SATAProgIF : u8 {
    VendorSpecific = 0x00,
    AHCI = 0x01,
    SerialStorageBus = 0x02
};

}

namespace Network {

enum class SubclassID : u8 { // controllers
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

}

namespace Display {

enum class SubclassID : u8 {
    VGA = 0x00,
    XGA = 0x01,
    ThreeD = 0x02,
};

enum class VGAProgIF : u8 {
    VGACompatibleController = 0x00, // mem 0xA0000-0xBFFFF, I/O 0x3B0-0x3BB, 0x3C0-0x3DF + aliases
    _8514Compatible = 0x01, // I/O addresses 0x2E8 and aliases 0x2Ea-0x2EF
};

}

namespace Multimedia {

enum class SubclassID : u8 {
    VideoDevice = 0x00,
    AudioDevice = 0x01,
    ComputerTelephonyDevice = 0x01,
    HDACompatibleAudioDevice = 0x3,
};

enum class HDACompatibleAudioDeviceProgIF : u8 {
    Compatible = 0x00,
    CompatibleWithVendorExtensions = 0x80,
};

}

namespace Memory {

enum class MemorySubclassID : u8 {
    RAM = 0x00,
    Flash = 0x01,
};

}

namespace Bridge {

enum class SubclassID : u8 {
    Host = 0x00,
    ISA = 0x01,
    PCI_TO_PCI = 0x4,
    CardBus = 0x07,
    PCI_TO_PCI_SemiTransparent = 0x09,
};

enum PCI_TO_PCIBridgeProgIF : u8 {
    Standard = 0x00,
    SupportsSubtractiveDecode = 0x01,
};

}

namespace SimpleCommunication {

enum class SubclassID : u8 {
    SerialController = 0x00,
};

enum class SerialControllerProgIF : u8 {
    _8250CompatibleGenericXT = 0x00,
    _16450Compatible = 0x00,
    _16550Compatible = 0x01,
    _16650Compatible = 0x02,
    _16750Compatible = 0x03,
    _16850Compatible = 0x04,
    _16950Compatible = 0x05,
};

}

namespace BaseSystemPeripheral {

enum class SubclassID : u8 {
    PIC = 0x00,
    DMAController = 0x01,
    Timer = 0x02,
    RTCController = 0x03,
    PCIHotplugController = 0x04,
    SDHostController = 0x5,
    IOMMU = 0x06
};

enum class PICProgIF : u8 {
    Generic8259Compatible = 0x00,
    ISACompatible = 0x01,
    EISACompatible = 0x02,
    IOAPICInterruptController = 0x10,
    IOxAPICInterruptController = 0x20,
};

enum class DMAControllerProgIF : u8 {
    Generic8237Compatible = 0x00,
    ISADMAController = 0x01,
    EISADMAController = 0x02,
};

enum class TimerProgIF : u8 {
    Generic8254Timer = 0x00,
    ISATimer = 0x01,
    EISATimers = 0x02,
    HPET = 0x03,
};

}

namespace InputDevice {

enum class SubclassID : u8 {
    KeyboardController = 0x00,
    DigitizerPen = 0x01,
    MouseController = 0x02,
    ScannerController = 0x03,
    GameportController = 0x04,
};

}

namespace SerialBus {

enum class SubclassID : u8 { // controllers
    FireWire = 0x00,
    USB = 0x03,
    SMBus = 0x05,
};

enum class USBProgIF : u8 {
    UHCI = 0x00,
    OHCI = 0x01,
    EHCI = 0x02, // USB2
    XHCI = 0x03, // USB3
    Unspecified = 0x80,
    USBDevice = 0xFE, // (not a host controller)
};

}

namespace Wireless {

enum class SubclassID : u8 {
    BluetoothController = 0x11,
    Ethernet802_1aController = 0x20, // 5 GHz
    Ethernet802_1bController = 0x21, // 2.4 GHz
};

}

}
