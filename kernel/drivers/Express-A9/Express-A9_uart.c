#include "drivers/uart.h"
#include "drivers/Express-A9/Express-A9_uart.h"
#include <stdint.h>


// Endereços Base para VExpress-A9 (UART PL011)
#define UART0_BASE 0x10009000

// Registradores Principais
#define UART0_DATA   ((volatile uint32_t *)(UART0_BASE + 0x00)) // Data Register
#define UART0_FLAGS   ((volatile uint32_t *)(UART0_BASE + 0x18)) // Flag Register

// Flags de Status
#define UART_FR_TXFF (1 << 5) // Transmit FIFO Full
#define UART_FR_RXFE (1 << 4) // Receive FIFO Empty

void k_setup_uart(){}

void k_uart_putc(char c) {
    // Aguarda enquanto o buffer de transmissão estiver cheio
    while (*UART0_FLAGS & UART_FR_TXFF);
    
    // Escreve o caractere no registrador de dados
    *UART0_DATA = c;
}

void k_uart_print(char* s){
    while(*s){
        k_uart_putc(*s++);
    }
}   

void k_uart_print_hex(unsigned int val){
    char hex_chars[] = "0123456789ABCDEF";
    k_uart_print("0x");
    for (int i = 28; i >= 0; i -= 4) {
        k_uart_putc(hex_chars[(val >> i) & 0xF]);
    }
}