#ifndef UART_H
#define UART_H

void k_setup_uart();
void k_uart_putc(char c);
void k_uart_print(char* s);
void k_uart_print_hex(unsigned int value);
void k_uart_printf(const char *fmt, ...);

void k_uart_print_no_interrupt(char* s);
void k_uart_printf_no_interrupt(const char *fmt, ...);
#endif // UART_H