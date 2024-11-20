#include "asm.h"
#include "lapic.h"
#include "pit.h"
#include "ioapic.h"

volatile u32 pitTicks;
u32 lapic_timer_ticks_per_ms;
u32 lapic_timer_divider;

// PIT_FREQUENCY (1193182) / 1000 ~ 1139 (~1ms)
#define PIT_CALIBRATION_RELOAD_VALUE 1139
#define CALIBRATION_MS_TO_WAIT  30

void calibrate_lapic_timer() {
    pitTicks = 0;

    // configure PIT to generate periodic interrupts
    pit_config cfg = (pit_config){
        .bcd = false,
        .mode = PIT_MODE_PERIODIC,
        .access_mode = PIT_ACCESS_LOHIBYTE,
        .channel = 0,
    };
    outb(PIT_MODE_COMMAND_PORT, *(u8*)&cfg);

    // send reload counter value to PIT
    u8 counter_low = (u8)PIT_CALIBRATION_RELOAD_VALUE;
    u8 counter_high = (u8)(PIT_CALIBRATION_RELOAD_VALUE >> 8);
    outb(PIT_CHANNEL_0_DATA_PORT, counter_low);
    outb(PIT_CHANNEL_0_DATA_PORT, counter_high);

    // stop LAPIC timer
    lapic_write_register(LAPIC_TIMER_INITIAL_COUNT_REG_OFFSET, 0);
    // setup LAPIC timer
    lapic_write_register(LAPIC_TIMER_DIVIDE_CONFIG_REG_OFFSET, LAPIC_TIMER_DIVIDER_2);

    // start PIT and LAPIC timer
    ioapic_setmask(IRQ_PIT, false);
    lapic_write_register(LAPIC_TIMER_INITIAL_COUNT_REG_OFFSET, (u32)-1);

    // wait for PIT to generate enough interrupts
    while(pitTicks < CALIBRATION_MS_TO_WAIT);

    // read LAPIC timer counter
    u32 current_lapic_count = lapic_read_register(LAPIC_TIMER_CURRENT_COUNT_REGISTER_OFFSET);

    // stop timers
    ioapic_setmask(IRQ_PIT, true);
    lapic_write_register(LAPIC_TIMER_INITIAL_COUNT_REG_OFFSET, 0);

    // calculate LAPIC calibrated ticks
    u32 lapic_timer_ticks_elapsed = ((u32)-1) - current_lapic_count;
    lapic_timer_ticks_per_ms = lapic_timer_ticks_elapsed / CALIBRATION_MS_TO_WAIT;
    lapic_timer_divider = LAPIC_TIMER_DIVIDER_2;
}

void start_lapic_timer(u32 initial_count, enum lapic_timer_divider divider) {
    lapic_write_register(LAPIC_TIMER_INITIAL_COUNT_REG_OFFSET, initial_count);
    lapic_write_register(LAPIC_TIMER_DIVIDE_CONFIG_REG_OFFSET, divider);
    lapic_timer_lvt_reg_t timer_lvt_entry = {
        .vector = IRQ_INTERRUPT_OFFSET + IRQ_LAPIC_TIMER,
        .mask = 0,
        .mode = LAPIC_TIMER_MODE_PERIODIC,
    };
    lapic_write_register(LAPIC_TIMER_LVT_REG_OFFSET, timer_lvt_entry.raw);
}

void pit_irq_handler() {
    pitTicks++;
}
