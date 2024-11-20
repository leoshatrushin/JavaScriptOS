#pragma once
#include "types.h"

typedef struct {
    u32 rsrv0 : 24;
    u8 lapic_id; // may only be 4 bits
} __attribute__((packed)) lapic_xapic_id_reg_t;
typedef u32 lapic_x2apic_id_reg_t;

// a special situation may occur when a processor raises its task priority to be >= the level of the
// interrupt for which the processor INTR signal is currently being asserted
// if at the time the INTA cycle is issued, the interrupt to be dispensed become masked by software,
// the LAPIC will deliver a spurious interrupt
// dispensing the interrupt does not affect the ISR, so the handler for this vector should return w/o EOI
typedef struct {
    u8  vector;
    u8  apic_software_enable : 1;
    u8  no_focus_processor_checking : 1; // when using the lowest-priority delivery mode
    u8  rsrv0 : 2;
    u8  no_eoi_broadcast : 1; // level-triggered interrupt EOI broadcasts to IOAPICs, default 0 (yes)
    u32 rsrv2 : 19;
} __attribute__((packed)) lapic_spurious_interrupt_vector_reg_t;

enum lapic_timer_mode {
    LAPIC_TIMER_MODE_ONESHOT = 0,
    LAPIC_TIMER_MODE_PERIODIC = 1,
    LAPIC_TIMER_MODE_TSC_DEADLINE = 2,
};

typedef union {
    struct {
    u8   vector : 8;
    u8   rsrv0 : 4;
    u8   delivery_status : 1;
    u8   rsrv1 : 3;
    u8   mask : 1;
    enum lapic_timer_mode mode : 2;
    u16   rsrv2 : 13;
    };
    u32 raw;
} __attribute__((packed)) lapic_timer_lvt_reg_t;

enum lapic_timer_divider {
    LAPIC_TIMER_DIVIDER_2 = 0,
    LAPIC_TIMER_DIVIDER_4 = 1,
    LAPIC_TIMER_DIVIDER_8 = 2,
    LAPIC_TIMER_DIVIDER_16 = 3,
    LAPIC_TIMER_DIVIDER_32 = 8,
    LAPIC_TIMER_DIVIDER_64 = 9,
    LAPIC_TIMER_DIVIDER_128 = 10,
    LAPIC_TIMER_DIVIDER_1 = 11,
};

#define LAPIC_ID_REG_OFFSET 0x20
#define LAPIC_SPURIOUS_INTERRUPT_VECTOR_REG_OFFSET 0xF0
// Local Vector Table (LVT) registers
// Specify interrupt delivery manner
#define LAPIC_TIMER_LVT_REG_OFFSET                0x320
#define LAPIC_TIMER_INITIAL_COUNT_REG_OFFSET      0x380
#define LAPIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET 0x390
#define LAPIC_TIMER_DIVIDE_CONFIG_REG_OFFSET      0x3E0
#define LAPIC_EOI_REG_OFFSET                      0xB0

void init_lapic(void);
u32 lapic_read_register(u32 offset);
void lapic_write_register(u32 offset, u32 value);
u32 lapicid(void);
void lapic_eoi(void);
boolean lapic_is_x2apic_mode(void);
