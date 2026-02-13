#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#include <stdbool.h>
/* Protótipos das Funções do Driver */

/**
 * @brief Inicializa o hardware PL050 (KMI) e habilita interrupções.
 */
void k_setup_ps2(void);

/**
 * @brief Handler de interrupção para o teclado (IRQ 44).
 * Deve ser registrado no GIC.
 */
void k_ps2_irq_handler(void);

/**
 * @brief Envia um comando de 8 bits para o teclado PS/2.
 * @param cmd O comando (ex: 0xED para LEDs).
 */
void k_ps2_write(uint8_t cmd);

/**
 * @brief Atualiza os LEDs físicos do teclado (Caps, Num, Scroll Lock).
 */
void k_ps2_update_leds(void);

#endif /* PS2_H */