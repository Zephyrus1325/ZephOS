#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void k_setup_timers();

extern volatile uint64_t system_ticks;

#endif // TIMER_H