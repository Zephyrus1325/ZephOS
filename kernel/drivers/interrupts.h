#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

void k_setup_interrupts();
void k_delay_ms(uint32_t ms);

extern void k_enable_interrupts();
extern void k_disable_interrupts();

//#include "drivers/Express-A9/Express-A9_interrupts.h"

#endif // INTERRUPTS_H