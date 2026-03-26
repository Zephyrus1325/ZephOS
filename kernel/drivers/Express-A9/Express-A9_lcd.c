#include <stdint.h>

#include "drivers/lcd.h"

#define LCD_BASE    0x10020000
#define LCD_TIM0    ((volatile uint32_t *)(LCD_BASE + 0x00)) // Horizontal timings
#define LCD_TIM1    ((volatile uint32_t *)(LCD_BASE + 0x04)) // Vertical timings
#define LCD_UPBASE  ((volatile uint32_t *)(LCD_BASE + 0x10)) // Endereço do Framebuffer
#define LCD_CTRL    ((volatile uint32_t *)(LCD_BASE + 0x18)) // Controle (ON/OFF/BPP)
#define LCD_WIDTH 1024
#define LCD_HEIGHT 768

// Alinhamento de 16 bytes é importante para o DMA do PL111
uint32_t framebuffer[LCD_WIDTH * LCD_HEIGHT] __attribute__((aligned(4096)));

void k_setup_lcd() {
    // 1. Configurar o endereço da RAM onde os pixels estão
    *LCD_UPBASE = (uint32_t)framebuffer;

    // 2. Configurar Timings (Exemplo para 1024x768)
    // Esses valores dependem da resolução. Para QEMU, valores simples funcionam:
    *LCD_TIM0 = 0x3F1F3F9C; // Valores mágicos para 1024x768 (VGA/VESA)
    *LCD_TIM1 = 0x090B61DF;

    // 3. Ativar o LCD
    // Bit 0: EN (Enable), Bit 1-3: BPP (101 para 24bpp ou 110 para 32bpp), Bit 5: LcdTft
    *LCD_CTRL = (1 << 0) | (6 << 1) | (1 << 5) | (1 << 11); 
}

void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < LCD_WIDTH && y >= 0 && y < LCD_HEIGHT) {
        framebuffer[y * LCD_WIDTH + x] = color;
    }
}

#include "drivers/uart.h"
#include "drivers/interrupts.h"

void clear_screen(uint32_t color) {
    //k_enable_interrupts();
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) { 
        framebuffer[i] = color;
    }
    //k_disable_interrupts();
}