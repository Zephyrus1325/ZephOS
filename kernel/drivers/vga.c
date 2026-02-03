#include "drivers/vga.h"
#include "uart.h"
#include <stdarg.h>               


#define VGA_PIXEL_BASE 0xC8000000
#define VGA_CHAR_BASE 0xC9000000
#define VGA_MAX_COL 80
#define VGA_MAX_ROW 60
#define VGA_WIDTH 320
#define VGA_HEIGHT 240

void vga_put_char(int x, int y, char c) {
    if (x < VGA_MAX_COL && y < VGA_MAX_ROW) {
        volatile char* vga_addr = (volatile char *)(VGA_CHAR_BASE + (y << 7) + x);
        *vga_addr = c;
    }
}

void vga_put_pixel(int x, int y, uint16_t c) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        volatile uint16_t* vga_addr = (volatile uint16_t *)(VGA_PIXEL_BASE + (y << 10) + (x << 1));
        *vga_addr = c;
    }
}

void vga_clear_screen() {
    for (int y = 0; y < VGA_MAX_ROW; y++) {
        for (int x = 0; x < VGA_MAX_COL; x++) {
            vga_put_char(x, y, ' ');
            
        }
    }
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_put_pixel(x, y, 0);
            
        }
    }
}

void vga_print_string(int x, int y, const char *str) {
    while (*str) {
        vga_put_char(x++, y, *str++);
        if (x >= VGA_MAX_COL) break; // Simples truncamento por enquanto
    }
}

#include <stdarg.h>

extern void console_putc(char c);
extern void disable_interrupts(); 
extern void enable_interrupts(); 
typedef void (*putc_func_t)(char);
extern void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args);

void vga_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    disable_interrupts();
    k_vprintf_internal(console_putc, fmt, args);
    enable_interrupts();
    
    va_end(args);
}