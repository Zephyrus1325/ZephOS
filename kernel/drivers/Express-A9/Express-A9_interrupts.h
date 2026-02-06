#ifndef EXPRESSS_A9_INTERRUPTS_H
#define EXPRESSS_A9_INTERRUPTS_H


void k_gic_init(void);
void k_config_interrupt(int N, void (*ISR)());
void k_irq_handler(void); // Chamada pelo assembly

#endif