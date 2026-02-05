#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// Endereços GIC VExpress-A9
#define GIC_CPU_BASE    0x10000100
#define GIC_DIST_BASE   0x10001000

#define GICC_IAR        ((volatile uint32_t *)(GIC_CPU_BASE + 0x00C))
#define GICC_EOIR       ((volatile uint32_t *)(GIC_CPU_BASE + 0x010))

void gic_init(void);
void config_interrupt(int N, void (*ISR)());
void k_irq_handler(void); // Chamada pelo assembly

#endif