#include "drivers/uart.h"
#include "drivers/timer.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "core/interrupts.h"
#include "core/task.h"
#include "core/memory.h"
#include "include/syscall.h"

#include <string.h>

extern void start_first_task(void);
extern void k_ps2_isr(void);
extern void timer_tick_handler();

// IDEIAS GERAIS
/*
    - Alterar o SHELL (ver abaixo)
    - Adicionar sistema de arquivos (FAT32?)
    - Melhorar o Driver VGA
    - Criar driver de Audio
    - Criar driver de mouse
    - Programas para fazer:
        - Editor de texto
        - Vizualizador de imagens
        - Sintetizador MIDI
        - Explorador de arquivos
    - Adicionar variáveis de ambiente
    - Criar interface gráfica
    - Implementar funções trigonométricas
    - Criar função random()
    - Adicionar níveis de permissão (user e admin)
    - Adicionar criação de usuário e senha
*/



// Instancia a tarefa atual como zero
tcb_t *current_task = 0;

void task_a(){
    char list[] = "123456789ABCFEF";
    char a = 0;
    while(1){
        //sys_write("well, this works?\n");
        if(a >= sizeof(list)-1){a = 0;}
        msleep(500);
    }
}  

void task_b(){
    char list[] = "123456789";
    int b = 0;
    uint8_t* base_ptr = k_malloc(sizeof(uint8_t) * 1000000);
    uart_printf("[Task B]: base_ptr = %x\n", base_ptr);
    while(1){
        //vga_put_char(10, 10, list[b++]);
        //vga_printf("%d\n", b++);
        //if(b >= sizeof(list)-1){b = 0;}
        //if(b < 1000000){
          //  base_ptr[b++] = 'D';
        //}
        msleep(100);
    }
}  

// TODO: Shell
/*
    - Comandos:
        - cd <directory>
        - dir 
        - mkdir <directory>
        - rmdir <directory>
        - rm <file>
        - run <file>
        - rename <file/directory> <name>
        - edit <file>
        - clear
        - process <param>
            - list
            - kill <process id>
    
    - Interpretador shell
        (newline representa fim de comando)
        - VARIABLES:
            VAR A = 0
            VAR B[4] = [0, 1, 2, 3]
        - IF-ELSE:
            IF(CONDITION){
                *CODE*
            } ELSE {
                *MORE CODE*
            }

        - FOR LOOP:
            FOR((VAR DEFINITION), (CONDITION), (INCREMENT)){
                *CODE*
            }

            FOR(VAR X = 2, X < 10, X++)
            
        - WHILE LOOP:
            WHILE(CONDITION){
                *CODE*
            }
        - FUNCTIONS:
            FUNCTION DO_THING(A, B){
                *CODE*
                RETURN 0 <- DEFAULT RETURN VALUE
            }
    
    - Coisas pra prestar atenção
        - converter todos os caracteres uppercase para lowercase na fase de interpretação
        - achar um jeito de interpretar diretórios e arquivos com espaços no nome (ignorar espaços ao final, se houver)
        - colocar o shell no user space
        - fazer com que o shell consiga procurar variáveis de ambiente do sistema

*/


extern int term_x;
extern int term_y;

void shell_task() {

    char cmd[64];
    int cmd_idx = 0;

    console_write("\n> ");

    //console_write("Kernel v1.0 Pronto.\n> ");

    while(1) {
        char c = sys_get_char(); // Pega do buffer circular do teclado
        if (c > 0) {
            if (c == '\b') {
                if (cmd_idx > 0) {
                    cmd_idx--;
                    console_putc('\b'); // console_putc apaga na VGA
                }
            } 
            else if (c == '\n') {
                cmd[cmd_idx] = '\0';
                console_putc('\n');
                
                // Processa o comando aqui
                //if (strcmp(cmd, "ps") == 0) {
                    //print_task_list(); // Função que você pode criar depois
                //} else if (strcmp(cmd, "clear") == 0) {
                    //vga_clear_screen();
                    //term_x = 0; term_y = 0;
                //}
                console_write("Command thought about :)\n\n");

                cmd_idx = 0;
                console_write("> ");
            } 
            else if (cmd_idx < 63) {
                cmd[cmd_idx++] = c;
                console_putc(c); // Ecoa o caractere
            }
        }
        msleep(10);
    }
}

void main() {
    uart_print("ZephOS v1.0 - Inicializando Kernel...\n");
    gic_init();
    config_interrupt(29, timer_tick_handler);
    config_interrupt(79, k_ps2_isr);
    timer_init(200000);     // Timer Interrupt a cada 1ms

    k_ps2_init();

    vga_clear_screen();
    k_console_write("ZephOS V1.0 - Booting...\n\n");
    k_heap_init();
    k_console_write("Heap: OK!\n");
    k_idle_task_init();
    k_console_write("Idle task: OK!\n");
    k_console_write("Configuring Initial Tasks: ");
    if(!k_task_create(task_a, 1024)){uart_print("ERROR CREATING TASK A\n");}
    if(!k_task_create(task_b, 1024)){uart_print("ERROR CREATING TASK B\n");}
    if(!k_task_create(shell_task, 1024)){uart_print("ERROR CREATING SHELL TASK\n");}
    k_console_write("OK!\n");

    k_console_printf("System stable. Free Heap: %d bytes\n", k_get_free_heap_no_interrupt());
    k_console_printf("%d processes running right now\n\n", task_count);
    current_task = &task_table[0]; // Define quem começa
    start_first_task();
    
}