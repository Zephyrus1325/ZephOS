#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"


void main(){
    k_setup_uart();
    k_setup_interrupts();
    k_setup_timers();
    k_enable_interrupts();
    k_uart_print("Hello World\n\r");
    while(1){
        k_uart_print("Hewo\n\r");
        k_delay_ms(1000);
    }
}