#ifndef INTERRUPTS_H
#define INTERRUPTS_H

// Configuração de interrupções
void gic_init(void);
void config_interrupt(int N, void (*ISR)());

// Controle de interrupções
extern void enable_interrupts(void); 
extern void disable_interrupts(void); 

#endif