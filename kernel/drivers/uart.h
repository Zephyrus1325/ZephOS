#ifndef UART_H
#define UART_H

#include <stdint.h>

// Endereço base da JTAG UART na DE1-SoC
#define JTAG_UART_BASE 0xFF201000

// Estrutura mapeada nos registradores da UART
typedef struct {
    volatile uint32_t data;    // Registro de Dados (R/W)
    volatile uint32_t control; // Registro de Controle (R/W)
} jtag_uart_t;

// Protótipos
void uart_putc(char c);
void uart_print(const char *str);
void uart_print_hex(uint32_t val);
void uart_printf(const char *fmt, ...);
#endif