#include <stdint.h>

// TODO: Padronizar isso aqui com a organização em struct do UART

// Endereços do ARM Private Timer
#define TIMER_BASE      0xFFFEC600
#define TIMER_LOAD      (TIMER_BASE + 0x00)
#define TIMER_COUNTER   (TIMER_BASE + 0x04)
#define TIMER_CONTROL   (TIMER_BASE + 0x08)
#define TIMER_INTERRUPT (TIMER_BASE + 0x0C)

void timer_clear_irq() {
    volatile uint32_t *timer_interrupt = (uint32_t *)TIMER_INTERRUPT;
    *timer_interrupt = 0x1; // Escrever 1 limpa a interrupção (W1C)
}

void timer_init(uint32_t load_value) {
    volatile uint32_t *timer_load = (uint32_t *)TIMER_LOAD;
    volatile uint32_t *timer_control = (uint32_t *)TIMER_CONTROL;
    volatile uint32_t *timer_interrupt = (uint32_t *)TIMER_INTERRUPT;

    *timer_control = 0;          // Para o timer antes de configurar
    *timer_load = load_value;    
    *timer_interrupt = 0x1;      // Limpa qualquer interrupção residual (W1C)
    *timer_control = 0x7;        // Liga: Enable(0), Auto-reload(1), IRQ-Enable(2)
    timer_clear_irq();           // Desliga qualquer interrupção anterior
}

extern void timer_clear_irq();
extern void k_tick_handler();
extern void scheduler();

// Função de Callback que o GIC chamará (ID 29)
void timer_tick_handler() {
    timer_clear_irq();
    k_tick_handler();
    scheduler();
}



