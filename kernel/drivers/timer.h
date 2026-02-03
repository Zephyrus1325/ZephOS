#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 * @brief Inicializa o ARM Private Timer.
 * @param load_value Valor de contagem inicial. 
 * Num sistema de 200MHz, 200.000.000 equivale a 1 segundo.
 */
void timer_init(uint32_t load_value);

/**
 * @brief Limpa a flag de interrupção do Timer.
 * Deve ser chamada obrigatoriamente dentro do Handler de IRQ.
 */
void timer_clear_irq();

/**
 * @brief Retorna o valor atual do contador (opcional).
 */
uint32_t timer_get_value();

#endif