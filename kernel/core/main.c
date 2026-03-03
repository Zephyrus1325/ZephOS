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
    sys_printf("Hello!\n\r", 0);
    k_uart_print("HEOWLO\n\r");
    while(1){
        sys_msleep(999);
    }
}

void task2(){
    while(1){
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

