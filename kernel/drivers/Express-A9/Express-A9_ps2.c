#include "drivers/ps2.h"
#include <stdint.h>
#include <stdbool.h>
#include "drivers/Express-A9/Express-A9_interrupts.h"
#include "core/fifo.h"

/* Endereços Base (VExpress-A9 MPCore mapeia no 0x10006000) */
#define KMI_BASE       0x10006000 
#define KMI_CR         ((volatile uint32_t *)(KMI_BASE + 0x00))
#define KMI_STAT       ((volatile uint32_t *)(KMI_BASE + 0x04))
#define KMI_DATA       ((volatile uint32_t *)(KMI_BASE + 0x08))

/* Bits de Status */
#define RX_FULL        (1 << 4)
#define TX_EMPTY       (1 << 6)

/* Comandos PS/2 */
#define PS2_CMD_LED    0xED
#define PS2_CMD_ACK    0xFA

/* Máquina de Estados do Driver */
static struct {
    bool shift;
    bool caps;
    bool ctrl;
    bool alt;
    bool is_break;    // Próximo byte é soltura de tecla (0xF0)
    bool is_extended; // Próximo byte é tecla especial (0xE0)
} kbd_state = {false};

/* Tabela Scancode Set 2 -> ASCII */
static const char scancode_map[] = {
    [0x1C] = 'a', [0x32] = 'b', [0x21] = 'c', [0x23] = 'd', [0x24] = 'e',
    [0x2B] = 'f', [0x34] = 'g', [0x33] = 'h', [0x43] = 'i', [0x3B] = 'j',
    [0x42] = 'k', [0x4B] = 'l', [0x3A] = 'm', [0x31] = 'n', [0x44] = 'o',
    [0x4D] = 'p', [0x15] = 'q', [0x2D] = 'r', [0x1B] = 's', [0x2C] = 't',
    [0x3C] = 'u', [0x2A] = 'v', [0x1D] = 'w', [0x22] = 'x', [0x35] = 'y',
    [0x1A] = 'z', [0x45] = '0', [0x16] = '1', [0x1E] = '2', [0x26] = '3',
    [0x25] = '4', [0x2E] = '5', [0x36] = '6', [0x3D] = '7', [0x3E] = '8',
    [0x46] = '9', [0x5A] = '\n', [0x29] = ' ', [0x66] = '\b', [0x0D] = '\t',
    [0x41] = ',', [0x49] = '.', [0x4A] = ';', [0x4E] = '-', [0x55] = '=',
    [0x0E] = '\''
};

static const char shift_map[] = {
    [0x1C] = 'A', [0x32] = 'B', [0x21] = 'C', [0x23] = 'D', [0x24] = 'E',
    [0x2B] = 'F', [0x34] = 'G', [0x33] = 'H', [0x43] = 'I', [0x3B] = 'J',
    [0x42] = 'K', [0x4B] = 'L', [0x3A] = 'M', [0x31] = 'N', [0x44] = 'O',
    [0x4D] = 'P', [0x15] = 'Q', [0x2D] = 'R', [0x1B] = 'S', [0x2C] = 'T',
    [0x3C] = 'U', [0x2A] = 'V', [0x1D] = 'W', [0x22] = 'X', [0x35] = 'Y',
    [0x1A] = 'Z', [0x45] = ')', [0x16] = '!', [0x1E] = '@', [0x26] = '#',
    [0x25] = '$', [0x2E] = '%', [0x36] = '^', [0x3D] = '&', [0x3E] = '*',
    [0x46] = '(', [0x41] = '<', [0x49] = '>', [0x4A] = ':', [0x4E] = '_',
    [0x55] = '+', [0x0E] = '\"'
};

/* Envia comando para o hardware do teclado */
void k_ps2_write(uint8_t data) {
    while (!(*KMI_STAT & TX_EMPTY)); // Busy wait pelo buffer de saída
    *KMI_DATA = data;
}

void k_ps2_update_leds() {
    k_ps2_write(PS2_CMD_LED);
    // Bit 2: Caps Lock, Bit 1: Num Lock, Bit 0: Scroll Lock
    k_ps2_write(kbd_state.caps << 2);
}

#include "drivers/uart.h"

/* Handler chamado pela interrupção IRQ 44 */
void k_ps2_irq_handler(void) {
        
    if (!(*KMI_STAT & RX_FULL)) return;

    uint8_t code = (uint8_t)(*KMI_DATA & 0xFF);

    // Trata prefixos
    if (code == 0xE0) {
        kbd_state.is_extended = true;
        return;
    }
    if (code == 0xF0) {
        kbd_state.is_break = true;
        return;
    }

    // Teclas Modificadoras
    if (code == 0x12 || code == 0x59) { // Shift Esquerdo ou Direito
        kbd_state.shift = !kbd_state.is_break;
    } 
    else if (code == 0x14) { // Ctrl
        kbd_state.ctrl = !kbd_state.is_break;
    }
    else if (code == 0x58 && !kbd_state.is_break) { // Caps Lock (Toggle no press)
        kbd_state.caps = !kbd_state.caps;
        k_ps2_update_leds();
    }

    // Processa tecla se for um "Make" (pressionar)
    if (!kbd_state.is_break) {
        char c = 0;
        
        // Lógica de Caps vs Shift
        bool upper = (kbd_state.shift != kbd_state.caps);

        if (upper && shift_map[code]) {
            c = shift_map[code];
        } else {
            c = scancode_map[code];
        }

        if (c) {
            // Aqui você chama sua função de Buffer Circular que criou para a UART
            
            k_fifo_put(c); 
        }
    }

    // Reseta estados de prefixo
    kbd_state.is_break = false;
    kbd_state.is_extended = false;
}

void k_setup_ps2() {
    // 1. Habilita KMI e interrupção de RX
    *KMI_CR = (1 << 2) | (1 << 4);
    // 2. Limpa qualquer lixo no buffer
    while (*KMI_STAT & RX_FULL) {
        volatile uint32_t dummy = *KMI_DATA;
    }
    // 3. Reset do teclado (opcional, envia 0xFF)
    k_ps2_write(0xFF);
    k_config_interrupt(44, k_ps2_irq_handler);
}