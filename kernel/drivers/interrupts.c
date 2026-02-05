#include "interrupts.h"

void (*ISR_fn[256])(void);

void gic_init(void) {
    // ICDPMR: Permite todas as prioridades
    *(volatile uint32_t *)(GIC_CPU_BASE + 0x004) = 0xF0;
    // ICCICR: Habilita interface do CPU
    *(volatile uint32_t *)(GIC_CPU_BASE + 0x000) = 1;
    // ICDDCR: Habilita o Distribuidor
    *(volatile uint32_t *)(GIC_DIST_BASE + 0x000) = 1;
}

void config_interrupt(int N, void (*ISR)(void)) {
    // ICDISERn: Habilitar interrupção (Set-Enable)
    int reg_offset = (N >> 5) * 4;
    int bit_index = N & 0x1F;
    *(volatile uint32_t *)(GIC_DIST_BASE + 0x100 + reg_offset) |= (1 << bit_index);

    // ICDIPTRn: Definir CPU alvo (CPU0 é bit 0)
    // Cada registro de 32 bits controla 4 interrupções (8 bits por interrupção)
    volatile uint8_t *target_reg = (volatile uint8_t *)(GIC_DIST_BASE + 0x800);
    target_reg[N] = 0x01; 

    ISR_fn[N] = ISR;
}

// Handler genérico chamado pelo Assembly
void k_irq_handler(void) {
    uint32_t id = *GICC_IAR; // Lê qual interrupção disparou

    if (id < 256 && ISR_fn[id]) {
        ISR_fn[id](); // Executa a função registrada
    }

    *GICC_EOIR = id; // Avisa o GIC que terminou
}