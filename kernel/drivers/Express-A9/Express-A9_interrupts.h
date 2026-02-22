#ifndef EXPRESSS_A9_INTERRUPTS_H
#define EXPRESSS_A9_INTERRUPTS_H

#include <stdint.h>

// Endereços GIC VExpress-A9
#define GIC_CPU_BASE    0x1e000100
#define GIC_DIST_BASE   0x1e001000

#define GICC_IAR        ((volatile uint32_t *)(GIC_CPU_BASE + 0x00C))
#define GICC_EOIR       ((volatile uint32_t *)(GIC_CPU_BASE + 0x010))

void k_config_interrupt(int N, void (*ISR)());
void k_irq_handler(void); // Chamada pelo assembly

/* Inicia o GIC */
inline void k_gic_init(void) {
    *(volatile uint32_t *)(GIC_CPU_BASE + 0x004) = 0xF0;    // ICDPMR: Permite todas as prioridades
    *(volatile uint32_t *)(GIC_CPU_BASE + 0x000) = 1;       // ICCICR: Habilita interface do CPU
    *(volatile uint32_t *)(GIC_DIST_BASE + 0x000) = 1;      // ICDDCR: Habilita o Distribuidor
}

// Habilita Interrupções
inline void k_enable_interrupts(){
    asm volatile ("cpsie i\n");
}

// Desabilita Interrupções
inline void k_disable_interrupts(){
    asm volatile ("cpsid i\n");
}

#endif