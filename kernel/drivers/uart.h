#ifndef UART_H
#define UART_H

void k_setup_uart();
void k_uart_putc(char c);
void k_uart_print(char* s);
void k_uart_print_hex(unsigned int value);
void k_uart_printf(char* s, ...);

#endif // UART_H