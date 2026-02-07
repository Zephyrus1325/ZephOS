#include "drivers/Express-A9/Express-A9_interrupts.h"
#include "drivers/interrupts.h"
#include "drivers/uart.h"
#include <stdint.h>

// Endereços GIC VExpress-A9
#define GIC_CPU_BASE    0x1e000100 //0x10000100
#define GIC_DIST_BASE   0x1e001000 //0x10001000

#define GICC_IAR        ((volatile uint32_t *)(GIC_CPU_BASE + 0x00C))
#define GICC_EOIR       ((volatile uint32_t *)(GIC_CPU_BASE + 0x010))

void (*ISR_fn[256])(void);

void k_setup_interrupts(){
    k_gic_init();
}

void k_gic_init(void) {
    // ICDPMR: Permite todas as prioridades
    *(volatile uint32_t *)(GIC_CPU_BASE + 0x004) = 0xF0;
    // ICCICR: Habilita interface do CPU
    *(volatile uint32_t *)(GIC_CPU_BASE + 0x000) = 1;
    // ICDDCR: Habilita o Distribuidor
    *(volatile uint32_t *)(GIC_DIST_BASE + 0x000) = 1;
}

void k_config_interrupt(int N, void (*ISR)(void)) {
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
    } else {
        k_uart_print("KERNEL: UNKNOWN IRQ CODE: 0x");
        k_uart_print_hex(id);
        k_uart_print("\n");
    }


    *GICC_EOIR = id; // Avisa o GIC que terminou
}

// Chamado pelo processador uma chamada de erro de dados
void k_panic_data_abort(uint32_t addr, uint32_t status, uint32_t pc) {
    k_uart_print("\n\rKERNEL: PANIC - DATA ABORT\n\r");
    k_uart_print("AT INSTRUCTION: ");
    k_uart_print_hex(pc);
    
    k_uart_print("\nTRIED TO ACCESS: ");
    k_uart_print_hex(addr);
    
    k_uart_print("\n\rStatus (DFSR): ");
    k_uart_print_hex(status);
    
    k_uart_print("\n\rSISTEM HALTED.");
    while(1); // Trava o sistema para debug
}