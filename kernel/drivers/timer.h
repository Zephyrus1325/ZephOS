#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void k_setup_timers();

extern volatile uint64_t system_ticks;
extern volatile uint32_t system_millis;

#endif // TIMER_H