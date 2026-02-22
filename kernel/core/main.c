#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "core/task.h"
#include "core/memory.h"
#include "include/syscall.h"
#include "drivers/ps2.h"

#include "drivers/lcd.h"

#include "drivers/sd.h"

extern void start_first_task(void);

void task1(){
    //k_uart_print("CARLOS\n\r");
    //k_uart_print("Hello World from task 5\n\r");
    //int b = 0;
    while(1){
        //sys_printf("1 - %d\n\r", b += 2);
        sys_msleep(100000);
    }
}

void task2(){
    //k_uart_print("CARLOS 2\n\r");
    //k_uart_print("Hello World     from task number 5\n\r");
    char c = 'a';
    while(1){
        int32_t temp = sys_getc();
        if(temp != -1){
            if(temp == '\b'){
                sys_write("\b ");
            }
            sys_putc((char)temp);
        }
        //sys_printf("%d\n\r", c);
        sys_msleep(10);
    }
}

void main(){
    k_setup_uart();
    k_uart_print("UART OK.\n\r");

    k_setup_interrupts();
    k_uart_print("INTERRUPTS OK.\n\r");

    k_setup_timers();
    k_uart_print("TIMERS OK.\n\r");

    k_setup_ps2();
    k_uart_print("PS/2 OK.\n\r");

    k_heap_init();
    k_uart_print("HEAP OK.\n\r");

    if (!k_sd_init()) {
        uint8_t sector0[512];
        k_sd_read_sector(0, sector0);
        if (sector0[510] == 0x55 && sector0[511] == 0xAA) {
            k_uart_print("SD CARD OK.\r\n");
        } else {
            k_uart_print("SD CARD - INVALID BOOT SECTOR ERROR.\r\n");
        }
    } else {
        k_uart_print("SD FAILED. \n\r");
    }

    k_setup_lcd();
    clear_screen(0x000000);
    k_uart_print("LCD DISPLAY OK.\n\r");

    k_idle_task_init();
    k_uart_print("IDLE TASK OK.\n\r");
    
    k_uart_print("\n\rSETUP COMPLETE.\n\r");
    k_task_create(task1, 1024);
    k_task_create(task2, 1024);
    k_uart_printf("tasks running: %d\n\r", task_count);
    k_uart_printf("free memory: %d bytes\n\r", k_get_free_heap_no_interrupt());
    k_enable_interrupts();
    current_task = &task_table[0]; // Define quem começa
    start_first_task();
}

