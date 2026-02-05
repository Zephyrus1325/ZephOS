#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_isr(void);
void timer_init(uint32_t ms_rate);
void delay_ms(uint32_t ms);

#endif