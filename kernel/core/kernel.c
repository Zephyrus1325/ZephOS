#include "drivers/uart.h"
#include <stdint.h>
#include <stddef.h>

// Tabela de handlers (Vetores de Software)
extern void (*ISR_fn[256])();

// Chamado pelo processador uma chamada de erro de dados
void k_panic_data_abort(uint32_t addr, uint32_t status, uint32_t pc) {
    uart_print("\nKERNEL: PANIC - DATA ABORT\n");
    uart_print("AT INSTRUCTION: ");
    uart_print_hex(pc);
    
    uart_print("\nTRIED TO ACCESS: ");
    uart_print_hex(addr);
    
    uart_print("\nStatus (DFSR): ");
    uart_print_hex(status);
    
    uart_print("\nSISTEM HALTED.");
    while(1); // Trava o sistema para debug
}

// Chamado por harware
void k_handle_irq(void) {
    // Lê o IAR (Interrupt Acknowledge Register)
    int irq_id = *(volatile int *)(0xFFFEC10C) & 0x3FF;

    if (irq_id < 256 && ISR_fn[irq_id] != 0) {
        ISR_fn[irq_id](); // Executa o handler específico
    } else {
        uart_print("KERNEL: UNKNOWN IRQ CODE: 0x");
        uart_print_hex(irq_id);
        uart_print("\n");
    }

    // Escreve no EOIR (End of Interrupt Register)
    *(volatile int *)(0xFFFEC110) = irq_id;
}