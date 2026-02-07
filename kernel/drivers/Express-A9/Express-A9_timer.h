#ifndef EXPRESSS_A9_TIMER_H
#define EXPRESSS_A9_TIMER_H

#include <stdint.h>

void k_timer_isr(void);
void k_timer_init(uint8_t timer_id, uint32_t trigger_value);

#endif