#pragma once
#include "lapic.h"

void calibrate_lapic_timer(void);
void start_lapic_timer(u32 initial_count, enum lapic_timer_mode mode);
