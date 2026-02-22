#include <stdint.h>
#include <stdbool.h>

#define FIFO_SIZE 256

static struct {
    uint8_t buffer[FIFO_SIZE];
    volatile int head; // Escrita (Kernel/IRQ)
    volatile int tail; // Leitura (User/Syscall)
} kbd_fifo = { .head = 0, .tail = 0 };

/* * Adiciona um caractere ao buffer.
 * Chamada pelo k_ps2_irq_handler ou k_uart_irq_handler.
 */
bool k_fifo_put(char c) {
    int next = (kbd_fifo.head + 1) % FIFO_SIZE;

    // Se o próximo índice for o tail, o buffer está cheio
    if (next == kbd_fifo.tail) {
        return false; // Caractere descartado (overflow)
    }

    kbd_fifo.buffer[kbd_fifo.head] = c;
    kbd_fifo.head = next;
    return true;
}

/* * Remove e retorna um caractere do buffer.
 * Chamada pela Syscall SYS_GETCHAR.
 */
int k_fifo_get(void) {
    // Se head == tail, o buffer está vazio
    if (kbd_fifo.head == kbd_fifo.tail) {
        return -1;
    }

    char c = kbd_fifo.buffer[kbd_fifo.tail];
    kbd_fifo.tail = (kbd_fifo.tail + 1) % FIFO_SIZE;
    return (int)c;
}

/* Retorna true se houver dados para ler */
bool k_fifo_has_data(void) {
    return kbd_fifo.head != kbd_fifo.tail;
}