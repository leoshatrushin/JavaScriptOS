// Programmable Interval Timer
// - one-shot + periodic interrupts
// - count-down
// advantages
// - spec-fixed frequency of 1.1931816666 MHz
// disadvantages
// - latency due to operating over port I/O
// - low frequency
// - 16-bit counter (short max period, lower precision over time due to rounding)

// From the original IBM PC, remains as a standard device emulated by the HPET until configured otherwise
// channels 1 and 2 were used for DRAM refresh and speaker, likely don't exist on modern systems

// by default appears in IRQ0, which may be remapped to IRQ2 on modern UEFI-based systems

// system-wide device, to use need to program the IOAPIC to route the interrupt to a LAPIC

#include "types.h"

#define PIT_FREQUENCY 1193182

#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_MODE_COMMAND_PORT   0x43

enum pit_mode {
    PIT_MODE_ONESHOT = 0,
    PIT_MODE_PERIODIC = 2,
};

enum pit_access_mode {
    PIT_ACCESS_LOHIBYTE = 3,
};

typedef struct {
    boolean              bcd         : 1;
    enum pit_mode        mode        : 3;
    enum pit_access_mode access_mode : 2;
    u8                   channel     : 2;
} pit_config;

#define APIC_TIMER_SET_PERIODIC 0x20000
#define APIC_TIMER_SET_MASKED   0x10000

void pit_irq_handler(void);
