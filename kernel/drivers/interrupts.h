#ifndef INTERRUPTS_H
#define INTERRUPTS_H

void k_setup_interrupts();

extern void k_enable_interrupts();
extern void k_disable_interrupts();

#include "drivers/Express-A9/Express-A9_interrupts.h"

#endif // INTERRUPTS_H