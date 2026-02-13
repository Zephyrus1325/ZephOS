#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "core/task.h"
#include "core/memory.h"
#include "include/syscall.h"

extern void start_first_task(void);

void task1(){
    //k_uart_print("CARLOS\n\r");
    //k_uart_print("Hello World from task 5\n\r");
    while(1){
        k_uart_print("Hello World from task 1\n\r");
        msleep(1000);
    }
}

void task2(){
    //k_uart_print("CARLOS 2\n\r");
    //k_uart_print("Hello World from task number 5\n\r");
    while(1){
        k_uart_print("Hello Again!\n\r");
        msleep(200);
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
    k_uart_printf("tasks running: %d\n\r", task_count);
    k_uart_printf("free memory: %d bytes\n\r", k_get_free_heap_no_interrupt());
    k_enable_interrupts();
    current_task = &task_table[0]; // Define quem começa
    start_first_task();
}

