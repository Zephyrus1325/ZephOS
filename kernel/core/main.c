#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "core/task.h"
#include "core/memory.h"
#include "include/syscall.h"

extern void start_first_task(void);

void task1(){
    k_uart_print("CARLOS\n\r");
    k_uart_print("Hello World from task 5\n\r");
    while(1){
        //k_uart_print("Hello World from task 2\n\r");
        //sys_msleep(1000);
    }
}

void task2(){
    k_uart_print("CARLOS 2\n\r");
    k_uart_print("Hello World from task number 5\n\r");
    while(1){
        //int32_t c = sys_getc();
        //
        //if(c != -1){
        //    k_uart_putc(c);
        //}
        ////k_uart_print("Hello Again!\n\r");
        //k_uart_print_hex(c);
        //k_uart_print("\n\r");
        sys_msleep(200);
    }
}

void main(){
    k_setup_uart();
    k_setup_interrupts();
    k_setup_timers();
    k_heap_init();
    k_idle_task_init();
    
    k_task_create(task1, 1024);
    k_task_create(task2, 1024);
    k_enable_interrupts();
    current_task = &task_table[0]; // Define quem começa
    start_first_task();
}

