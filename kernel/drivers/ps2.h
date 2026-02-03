#ifndef PS2_H
#define PS2_H

void k_ps2_init();
void k_ps2_isr();
void k_kbd_push(char c);
char sys_get_char();
void console_putc(char c);
void console_write(const char *str);
void k_console_write(const char *str);
void k_console_printf(const char *fmt, ...);
#endif