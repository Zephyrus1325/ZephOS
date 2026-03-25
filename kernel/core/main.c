#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "core/task.h"
#include "core/memory.h"
#include "include/syscall.h"
#include "drivers/ps2.h"

#include "drivers/interrupts.h"
#include "drivers/lcd.h"

#include "drivers/sd.h"
#include "core/filesystem.h"

extern void start_first_task(void);
extern void k_uart_printf_no_interrupt(const char *fmt, ...);
extern void k_uart_print_no_interrupt(char* s);

extern int getpid(void);

void task1(void){
    char buffer[512];
    FILE *fp;
    int count = 0;

    printf("--- Iniciando Task1 (User Mode) ---\n\r");
    // 1. Tentar abrir um arquivo no SD Card
    printf("Tentando abrir o arquivo...\n\r");
    fp = fopen("grande.txt", "r");
    printf("Address do arquivo: 0x%x\n\r", (uint32_t) fp);
    if (fp == NULL) {
        printf("Erro: Nao foi possivel abrir TEST.TXT\n\r");
        // Se falhar, entra em loop de erro
        while(1) { msleep(1000);}
    }

    printf("Arquivo aberto com sucesso! Lendo conteudo...\n\r");

    // 2. Ler e imprimir o conteúdo linha por linha ou bloco
    // Usando o fread padrão: ptr, size, nmemb, stream
    int temp;
    int total = 0;
    while ((temp = fread(buffer, 1, 511, fp)) > 0) {
        buffer[511] = '\0'; // Garante o null-terminator para o printf
        //printf("---- Chunk: [%d] ----\n\r%s", count, buffer);
        //printf("%s", buffer);
        total += temp;
        count++;
        if(count > 100) break; // Segurança contra loop infinito
    }

    printf("Total lido: %d bytes\n\r", total);

    // 3. Fechar o arquivo
    fclose(fp);
    msleep(1000);
    printf("Arquivo fechado. Task1 finalizada.\n\r");

    // 4. Loop de idle da tarefa
    printf("Task1 rodando... PID: %d\n", getpid());
    while (1) {
        //printf("TESTE A\n\r");
        msleep(1000); // Dorme por 2 segundos
    }
}

void task2(){
    //printf("Task2 rodando... PID: %d\n", getpid());
    while(1){
        //printf("TESTE B\n\r");
        msleep(20000);
    }
}


void main(){
    uint8_t init = 1;

    k_setup_uart();
    k_uart_print_no_interrupt("[KERNEL]: UART OK.\n\r");

    k_setup_ps2();
    k_uart_print_no_interrupt("[KERNEL]: PS/2 OK.\n\r");

    k_heap_init();
    k_uart_print_no_interrupt("[KERNEL]: HEAP OK.\n\r");

    if(!k_fs_init()){k_uart_print_no_interrupt("[KERNEL]: FILESYSTEM OK.\n\r");}
    else{k_uart_print_no_interrupt("[KERNEL]: FILESYSTEM FAILED. [!!!]\n\r"); init = 0;}

    k_setup_lcd();
    clear_screen(0x000000);
    k_uart_print_no_interrupt("[KERNEL]: LCD DISPLAY OK.\n\r");

    k_idle_task_init();
    k_uart_print_no_interrupt("[KERNEL]: IDLE TASK OK.\n\r");

    k_setup_interrupts();
    k_uart_print_no_interrupt("[KERNEL]: INTERRUPTS OK.\n\r");

    k_setup_timers();
    k_uart_print_no_interrupt("[KERNEL]: TIMERS OK.\n\r");
    
    if(init){
        k_uart_print_no_interrupt("\n\r[KERNEL]: SETUP COMPLETE.\n\r");
    } else {
        k_uart_print_no_interrupt("\n\r[KERNEL]: SETUP FAILED.\n\r");
        while(1){;}
    }

    k_task_create_no_interrupt(task1, 1024);
    k_task_create_no_interrupt(task2, 1024);
    k_uart_printf_no_interrupt("[KERNEL]: TASKS RUNNING: %d\n\r", task_count);
    k_uart_printf_no_interrupt("[KERNEL]: FREE MEMORY: %d BYTES\n\n\r", k_get_free_heap_no_interrupt());
    current_task = &task_table[0]; // Define quem começa
    start_first_task();     // Configura a primeira tarefa corretamente e ativa interrupções
}

