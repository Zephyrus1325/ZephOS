#include "uart.h"
#include <stdarg.h>

void uart_putc(char c) {
    // Aponta para a estrutura no endereço de memória da UART
    jtag_uart_t *uart = (jtag_uart_t *)JTAG_UART_BASE;

    // O campo WSPACE (espaço de escrita) fica nos 16 bits superiores do registro control
    // Precisamos garantir que WSPACE > 0 antes de enviar
    while (((uart->control >> 16) & 0xFFFF) == 0);

    // Escreve o caractere no registro de dados (bits 0-7)
    uart->data = (uint8_t)c;
}

void uart_print(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}

// Função simples para converter número em Hexadecimal e imprimir
void uart_print_hex(uint32_t val) {
    char hex_chars[] = "0123456789ABCDEF";
    uart_print("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex_chars[(val >> i) & 0xF]);
    }
}

#include <stdarg.h>

extern void disable_interrupts(); 
extern void enable_interrupts(); 
typedef void (*putc_func_t)(char);
extern void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args);

void uart_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // UART geralmente é atômica por caractere, mas para mensagens 
    // completas não misturarem, mantemos o lock.
    disable_interrupts();
    k_vprintf_internal(uart_putc, fmt, args);
    enable_interrupts();
    
    va_end(args);
}