#include "drivers/Express-A9/Express-A9_interrupts.h"
#include "drivers/interrupts.h"
#include "drivers/uart.h"
#include <stdint.h>

void (*ISR_fn[256])(void);  // Array com os ISRs de cada ID de interrupção


/* Configura Interrupções do sistema */
void k_setup_interrupts(){
    k_disable_interrupts(); // Garante que o interrupt disable está ativado (evita de ativar uma interrupção durante o setup)
    k_gic_init();
}


/* 
    Configura uma nova fonte de interrupção 

    @param N ID da interrupção
    @param ISR Função que trata desta interrupção

*/
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


/* Handler de interrupções chamado pelo assembly */
void k_irq_handler(void) {
    uint32_t id = *GICC_IAR; // Lê qual interrupção disparou
    if (id < 256 && ISR_fn[id]) {
        ISR_fn[id](); // Executa a função registrada
    } else {
        k_uart_print("[KERNEL]: UNKNOWN IRQ CODE: 0x");
        k_uart_print_hex(id);
        k_uart_print("\n");
    }

    *GICC_EOIR = id; // Avisa o GIC que terminou
}


/* 
    Handler de erro de dados, chamado pelo processador 

    @param addr Endereço que tentou ser acessado
    @param status valor do status register no momento do erro
    @param pc Endereço do código que causou o erro
*/
void k_panic_data_abort(uint32_t addr, uint32_t status, uint32_t pc) {
    k_uart_printf("\n\r[KERNEL]: PANIC - DATA ABORT\n\r"
        "\n\rAT INSTRUCTION: 0x%x\n\r"
        "\n\rTRIED TO ACCESS: 0x%x\n\r"
        "\n\rStatus (DFSR): 0x%x\n\r"
        "\n\rSISTEM HALTED.\n\r"
        , pc, addr, status);
    while(1); // Trava o sistema para debug
}



