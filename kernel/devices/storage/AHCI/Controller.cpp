#include "Controller.hpp"

using namespace Kernel::AHCI;

AHCIController::AHCIController(const PCI::config_space_address& base) : PCI::Device{base} {
    this->hba_regs = static_cast<HBARegisters*>(TRYHALT(BAR_address(AHCI_Base_Memory_Register_BARNO)));
    this->hba_regs->control_regs.global_host_control.ahci_enable = 1;
    this->command_reg()->bus_master = 1;
}
