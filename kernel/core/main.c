#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "core/task.h"
#include "core/memory.h"
#include "include/syscall.h"
#include "drivers/ps2.h"

#include "drivers/lcd.h"

#include "drivers/sd.h"
#include "core/filesystem.h"

extern void start_first_task(void);

void task1(){
    k_uart_print("HEY!\n\rArquivos!\n\r");
    
    file_t f;
    int status = k_fs_open("teste.txt", &f);
    k_uart_printf("%d\n\r", status);
    k_uart_printf("0x%d\n\r", (unsigned int)f.buffer);
    //if(k_fs_open("teste.txt", &f)){
    //    k_uart_print("Abri o arquivo...\n\r");
    //    k_uart_printf("0x%x\n\r", f.buffer);
    //} else {
    //    k_uart_print("NAO FOI AAAAAAAA\n\r");
    //}
    

    
    while(1){
        sys_printf("[PROG A]: E\n\r");
        sys_msleep(999);
    }
}

void task2(){
    
    while(1){
        //sys_printf("[PROG B]: %d\n\r", b++);
        sys_msleep(1000099);
    }
}

void main(){
    uint8_t init = 1;

    k_setup_uart();
    k_uart_print("[KERNEL]: UART OK.\n\r");

    k_setup_interrupts();
    k_uart_print("[KERNEL]: INTERRUPTS OK.\n\r");

    k_setup_timers();
    k_uart_print("[KERNEL]: TIMERS OK.\n\r");

    k_setup_ps2();
    k_uart_print("[KERNEL]: PS/2 OK.\n\r");

    k_heap_init();
    k_uart_print("[KERNEL]: HEAP OK.\n\r");

    if(!k_fs_init()){k_uart_print("[KERNEL]: FILESYSTEM OK.\n\r");}
    else{k_uart_print("[KERNEL]: FILESYSTEM FAILED. [!!!]\n\r"); init = 0;}
    

    k_setup_lcd();
    clear_screen(0x000000);
    k_uart_print("[KERNEL]: LCD DISPLAY OK.\n\r");

    k_idle_task_init();
    k_uart_print("[KERNEL]: IDLE TASK OK.\n\r");


    if(init){
        k_uart_print("\n\r[KERNEL]: SETUP COMPLETE.\n\r");
    } else {
        k_uart_print("\n\r[KERNEL]: SETUP FAILED.\n\r");
        while(1){;}
    }
    k_task_create_no_interrupt(task1, 1024);
    k_task_create_no_interrupt(task2, 1024);
    k_uart_printf("tasks running: %d\n\r", task_count);
    k_uart_printf("free memory: %d bytes\n\n\r", k_get_free_heap_no_interrupt());
    k_enable_interrupts();
    current_task = &task_table[0]; // Define quem começa
    start_first_task();
}

