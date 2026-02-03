#include "interrupts.h"
#include "drivers/uart.h"
#include <stdint.h>


// TODO: Tirar numeros Magicos 

void (*ISR_fn[256])();

void gic_init(void) {
    // Permite todas as prioridades
    *(volatile int *)0xFFFEC104 = 0xFFFF;
    // Habilita interface do CPU
    *(volatile int *)0xFFFEC100 = 1;
    // Habilita o Distribuidor
    *(volatile int *)0xFFFED000 = 1;
}

void config_interrupt(int N, void (*ISR)()) {
    int reg_offset, index, value, address;

    // 1. Habilitar a interrupção no Distribuidor (ICDISERn)
    reg_offset = (N >> 5) * 4; // Divisão por 32 para achar o registro
    index = N & 0x1F;          // N mod 32
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    *(volatile int *)address |= value;

    // 2. Definir o processador alvo (ICDIPTRn) - CPU0
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;
    *(volatile char *)address = (char)1;

    ISR_fn[N] = ISR; // Guarda a função na nossa tabela
}

