#include "drivers/interrupts.h"
#include "drivers/timer.h"
#include "drivers/uart.h"


void main(){
    gic_init();
    timer_init(10000);
    k_uart_print("Hello World\n\r");
    while(1){
        k_uart_print("Hewo\n\r");
        delay_ms(1000);
    }
}