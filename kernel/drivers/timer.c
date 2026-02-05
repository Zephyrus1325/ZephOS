#include "interrupts.h"

#define TIMER0_BASE 0x10011000
#define TIMER_LOAD    ((volatile uint32_t *)(TIMER0_BASE + 0x00))
#define TIMER_INTCLR  ((volatile uint32_t *)(TIMER0_BASE + 0x0C))
#define TIMER_CONTROL ((volatile uint32_t *)(TIMER0_BASE + 0x08))

volatile uint32_t system_ticks = 0;

void timer_isr(void) {
    *TIMER_INTCLR = 1; // Limpa interrupção no hardware do timer
    system_ticks++;
}

void timer_init(uint32_t ms_rate) {
    // Configura 1ms: 1MHz / 1000 = 1000 ticks
    *TIMER_LOAD = 1000; 
    
    // Timer Control: Enable (7), Periodic (6), Int Enable (5), 32-bit (1)
    *TIMER_CONTROL = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 1);

    config_interrupt(34, timer_isr);
}

void delay_ms(uint32_t ms) {
    uint32_t start = system_ticks;
    while ((system_ticks - start) < ms) {
        // Espera ocupada (ou WFI para economizar CPU)
        __asm__("wfi"); 
    }
}