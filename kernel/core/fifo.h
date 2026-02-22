#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Insere um caractere no buffer circular do teclado.
 * @param c Caractere vindo do driver de hardware.
 * @return true se inserido, false se o buffer estiver cheio.
 */
bool k_fifo_put(char c);

/**
 * @brief Retira um caractere do buffer circular.
 * @return O caractere (0-255) ou -1 se o buffer estiver vazio.
 */
int k_fifo_get(void);

/**
 * @brief Verifica se existem dados pendentes no buffer.
 * @return true se houver caracteres para ler.
 */
bool k_fifo_has_data(void);

#endif /* FIFO_H */