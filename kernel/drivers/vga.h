#ifndef VGA_H
#define VGA_H

// TODO:
/*
*/


#include <stdint.h>

void vga_put_char(int x, int y, char c);
void vga_put_pixel(int x, int y, uint16_t c);
void vga_clear_screen();
void vga_print_string(int x, int y, const char *str);
void vga_printf(const char *fmt, ...);
#endif