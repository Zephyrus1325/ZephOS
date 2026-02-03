#include "drivers/ps2.h"
#include "drivers/uart.h"
#include <stdint.h>

#define PS2_DATA    ((volatile uint32_t *) 0xFF200100)
#define PS2_CONTROL ((volatile uint32_t *) 0xFF200104)

int term_x;
int term_y;

void k_ps2_init() {
    // Habilita a interrupção no controlador PS/2
    *PS2_CONTROL = 0x1; 
    term_x = 3;
    term_y = 2;
}

// Tabela de mapeamento Scan Code -> ASCII (Sem Shift)
static const char ps2_map_no_shift[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', 0,          // 00-0F
    0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0, // 10-1F
    0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0, // 20-2F
    0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0, // 30-3F
    0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0, // 40-4F
    0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, '\n', ']', 0, '\\', 0, 0,        // 50-5F
    0, 0, 0, 0, 0, 0, '\b', 0, 0, 0, 0, 0, 0, 0, 0, 0         // 60-6F (0x66 = Backspace)
};

// Tabela de mapeamento Scan Code -> ASCII (Com Shift)
static const char ps2_map_shift[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', 0,
    0, 0, 0, 0, 0, 'Q', '!', 0, 0, 0, 'Z', 'S', 'A', 'W', '@', 0,
    0, 'C', 'X', 'D', 'E', '$', '#', 0, 0, ' ', 'V', 'F', 'T', 'R', '%', 0,
    0, 'N', 'B', 'H', 'G', 'Y', '^', 0, 0, 0, 'M', 'J', 'U', '&', '*', 0,
    0, '<', 'K', 'I', 'O', ')', '(', 0, 0, '>', '?', 'L', ':', 'P', '_', 0,
    0, 0, '"', 0, '{', '+', 0, 0, 0, 0, '\n', '}', 0, '|', 0, 0,
    0, 0, 0, 0, 0, 0, '\b', 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int is_break = 0;    // Indica que a próxima tecla recebida foi solta
static int shift_held = 0;  // Indica se o Shift está pressionado

char process_ps2_scancode(uint8_t code) {
    // 1. Verificar se é o prefixo de "Break" (tecla solta)
    if (code == 0xF0) {
        is_break = 1;
        return 0;
    }

    // 2. Verificar se é Shift (Left: 0x12, Right: 0x59)
    if (code == 0x12 || code == 0x59) {
        shift_held = !is_break; // Se for break, soltou. Se não, pressionou.
        is_break = 0;
        return 0;
    }

    // 3. Se for um "Break" de qualquer outra tecla, apenas limpamos o flag
    if (is_break) {
        is_break = 0;
        return 0;
    }

    // 4. Tradução usando as tabelas
    char ascii = 0;
    if (code < sizeof(ps2_map_no_shift)) {
        ascii = shift_held ? ps2_map_shift[code] : ps2_map_no_shift[code];
    }

    return ascii;
}

#define KBD_BUFFER_SIZE 128
static char kbd_buffer[KBD_BUFFER_SIZE];
static int head = 0, tail = 0;

void k_kbd_push(char c) {
    int next = (head + 1) % KBD_BUFFER_SIZE;
    if (next != tail) { // Se não estiver cheio
        kbd_buffer[head] = c;
        head = next;
    }
}

void k_ps2_isr() {
    uint32_t data_reg = *PS2_DATA;
    
    if (data_reg & 0x8000) { // RVALID
        uint8_t scancode = (uint8_t)(data_reg & 0xFF);
        char ascii = process_ps2_scancode(scancode);
        
        if (ascii != 0) {
            k_kbd_push(ascii); // Coloca no buffer para o Shell ler
        }
    }
}

char sys_get_char() {
    if (head == tail) return 0; // Buffer vazio
    char c = kbd_buffer[tail];
    tail = (tail + 1) % KBD_BUFFER_SIZE;
    return c;
}

#define VGA_CHAR_BASE 0xC9000000
#define START_COL 3
#define START_ROW 5
#define MAX_COLS 80
#define MAX_ROWS 60



extern void disable_interrupts(); 
extern void enable_interrupts(); 

void vga_scroll() {
    // Usamos uint32_t para garantir cópias de 4 bytes por vez (mais eficiente)
    volatile uint32_t *vga_ptr = (volatile uint32_t *)VGA_CHAR_BASE;
    
    // Cada linha na memória da VGA ocupa exatamente 128 bytes.
    // Como estamos usando uint32_t (4 bytes), cada linha tem 32 "words".
    const int words_per_stride = 32; 

    for (int y = 0; y < MAX_ROWS - 1; y++) {
        // Calculamos o início da linha atual e da próxima
        int current_line_offset = y * words_per_stride;
        int next_line_offset = (y + 1) * words_per_stride;

        for (int x = 0; x < words_per_stride; x++) {
            vga_ptr[current_line_offset + x] = vga_ptr[next_line_offset + x];
        }
    }

    // Limpa a última linha (Linhha 59)
    // Usamos char aqui para garantir que limpamos apenas as 80 colunas visíveis
    volatile char *last_row = (volatile char *)(VGA_CHAR_BASE + ((MAX_ROWS - 1) * 128));
    for (int x = 0; x < MAX_COLS; x++) {
        last_row[x] = ' ';
    }
}

// Função interna para colocar um caractere e avançar o cursor
void console_putc(char c) {
    // Seção Crítica: Impede que duas tasks baguncem a posição do cursor ao mesmo tempo
    //disable_interrupts(); 

    if (c == '\n') {
        term_x = START_COL;
        term_y++;
    } else if (c == '\b') {
        if (term_x > 0) {
            term_x--;
            // Apaga o caractere visualmente movendo o cursor e escrevendo espaço
            volatile char *vga_addr = (volatile char *)(VGA_CHAR_BASE + (term_y << 7) + term_x);
            *vga_addr = ' ';
        }
    } else if (c >= 32 && c <= 126) { // Caracteres imprimíveis
        volatile char *vga_addr = (volatile char *)(VGA_CHAR_BASE + (term_y << 7) + term_x);
        *vga_addr = c;
        term_x++;

        // Se estourar a largura da tela, faz o wrap (quebra de linha automática)
        if (term_x >= MAX_COLS) {
            term_x = START_COL;
            term_y++;
        }
    }

    // VERIFICAÇÃO DE SCROLL: Se a nova posição Y saiu da tela
    if (term_y >= MAX_ROWS-3) {
        vga_scroll();
        term_y = MAX_ROWS - 4; // Mantém o cursor na última linha
    }

    //enable_interrupts();
}



void console_write(const char *str) {
    // Como múltiplas tarefas podem querer escrever, desative interrupções ou use um Mutex
    disable_interrupts();
    while (*str) {
        console_putc(*str++);
    }
    enable_interrupts();
}

void k_console_write(const char *str){
    while (*str) {
        console_putc(*str++);
    }
}

#include <stdarg.h>
typedef void (*putc_func_t)(char);
extern void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args);

void k_console_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    k_vprintf_internal(console_putc, fmt, args);
    
    va_end(args);
}
