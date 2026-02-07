#include "drivers/timer.h"
#include "drivers/Express-A9/Express-A9_interrupts.h"
#include "drivers/Express-A9/Express-A9_timer.h"
#include <stdint.h>


#include "drivers/uart.h"

// All SP804 Timer Registers
#define SP804_TIMER_BASE 0x10011000

#define SP804_TIMER1_LOAD    ((volatile uint32_t*)(SP804_TIMER_BASE + 0x00))
#define SP804_TIMER1_VALUE   ((volatile uint32_t*)(SP804_TIMER_BASE + 0x04))
#define SP804_TIMER1_CONTROL ((volatile uint32_t*)(SP804_TIMER_BASE + 0x08))
#define SP804_TIMER1_INTCLR  ((volatile uint32_t*)(SP804_TIMER_BASE + 0x0C))
#define SP804_TIMER1_RIS     ((volatile uint32_t*)(SP804_TIMER_BASE + 0x10))
#define SP804_TIMER1_MIS     ((volatile uint32_t*)(SP804_TIMER_BASE + 0x14))
#define SP804_TIMER1_BGLOAD  ((volatile uint32_t*)(SP804_TIMER_BASE + 0x18))

#define SP804_TIMER2_LOAD    ((volatile uint32_t*)(SP804_TIMER_BASE + 0x20))
#define SP804_TIMER2_VALUE   ((volatile uint32_t*)(SP804_TIMER_BASE + 0x24))
#define SP804_TIMER2_CONTROL ((volatile uint32_t*)(SP804_TIMER_BASE + 0x28))
#define SP804_TIMER2_INTCLR  ((volatile uint32_t*)(SP804_TIMER_BASE + 0x2C))
#define SP804_TIMER2_RIS     ((volatile uint32_t*)(SP804_TIMER_BASE + 0x30))
#define SP804_TIMER2_MIS     ((volatile uint32_t*)(SP804_TIMER_BASE + 0x34))
#define SP804_TIMER2_BGLOAD  ((volatile uint32_t*)(SP804_TIMER_BASE + 0x38))

#define TIMER_EN (1<<7)
#define TIMER_PERIODIC (1<<6)
#define TIMER_INT (1<<5)
#define TIMER_SIZE (1<<1)
#define TIMER_ONESHOT (1<<0)

volatile uint64_t system_ticks = 0;
extern void k_tick_handler(void);
extern void scheduler(void);

void k_setup_timers(){
    // Timer 0 configurado para dar trigger a cada 1ms
    k_timer_init(0, 1000);
}

void k_timer1_isr(void) {
    *SP804_TIMER1_INTCLR = 1; // Limpa interrupção no hardware do timer
    system_ticks++;
    k_tick_handler();
    scheduler();
}

void k_timer2_isr(void) {
    *SP804_TIMER2_INTCLR = 1; // Limpa interrupção no hardware do timer
    // Faz nada :/
}

void k_timer_init(uint8_t timer_id, uint32_t trigger_value) {
    
    if(!timer_id){
        // Timer 1
        // Coloca o valor de load no timer
        *SP804_TIMER1_LOAD = trigger_value;
        // Timer Control: Enable (7), Periodic (6), Int Enable (5), 32-bit (1)
        *SP804_TIMER1_CONTROL = TIMER_EN | TIMER_PERIODIC | TIMER_INT | TIMER_SIZE;
        k_config_interrupt(34, k_timer1_isr);
    } else {
        // Timer 2
        // Coloca o valor de load no timer
        *SP804_TIMER2_LOAD = trigger_value;

        // Timer Control: Enable (7), Periodic (6), Int Enable (5), 32-bit (1)
        *SP804_TIMER2_CONTROL = TIMER_EN | TIMER_PERIODIC | TIMER_INT | TIMER_SIZE;
        k_config_interrupt(34, k_timer2_isr);
    }
    
}

void k_delay_ms(uint32_t ms) {
    uint32_t start = system_ticks;
    while ((system_ticks - start) < ms) {
        // Espera ocupada (ou WFI para economizar CPU)
        __asm__("wfi"); 
    }
}