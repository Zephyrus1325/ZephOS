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

void task1(){

    //uint8_t* meu_log = k_malloc(1024);
    //// Copia uma string para o buffer (simulando dados)
    //char *msg = "LOG DO KERNEL: Sistema de arquivos funcionando!";
    //for(int i = 0; msg[i] != '\0'; i++) meu_log[i] = msg[i];
//
    //if (k_fs_save_file("KERNAL.LOG", meu_log, 46) == 0) {
    //    k_uart_print("Teste de persistencia OK. Verifique no host com mdir.\r\n");
    //}
    
    while(1){
        //sys_printf("[PROG A]: E\n\r");
        sys_msleep(999);
    }
}

void task2(){
    k_uart_print("HEY!\n\rArquivos TAMBEM!!!!!\n\r");
    
    file_t f;
    k_uart_printf("[PROG B]: HEAP BEFORE FILE READ: %d BYTES\n\r", k_get_free_heap());
    file_t f2;

    if(!k_fs_open("kernal.log", &f)){
        k_uart_printf("[PROG B]: BUFFER COISO 2: 0x%x\n\r", (unsigned int) f.buffer);
        for(int i = 0; i < f.size; i++){
            k_uart_putc(f.buffer[i]);
        }
    } else {
        k_uart_print("\n\rNAO FOI 2 AAAAAAAA\n\r");
    }

    if(!k_fs_open("kernel.log", &f2)){
        k_uart_printf("[PROG B]: BUFFER COISO 2: 0x%x\n\r", (unsigned int) f2.buffer);
        for(int i = 0; i < f2.size; i++){
            k_uart_putc(f2.buffer[i]);
        }
    } else {
        k_uart_print("\n\rNAO FOI 2 AAAAAAAA\n\r");
    }

    k_uart_printf("\n\r[PROG B]: HEAP AFTER FILE READ: %d BYTES\n\r", k_get_free_heap());
    while(1){
        //sys_printf("[PROG B]: %d\n\r", b++);
        sys_msleep(1000099);
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
    k_enable_interrupts();
    current_task = &task_table[0]; // Define quem começa
    start_first_task();
}

