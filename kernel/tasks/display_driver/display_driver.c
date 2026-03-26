#include "display_driver.h"
#include "include/syscall.h"

#include "drivers/lcd.h"

/*
 *  Task Responsável por criar todo os gráficos básicos do sistema
 *  Boa parte das coisas do so devem ser um service para esta task
 */

static void main(){
    
    for(int y = 0; y < 200; y++){
        for(int x = 0; x < 300; x++){
            uint32_t color = (x % 50) < 25 ? 0 : 0xFFFFFFFF;//(((y * 0xff) / (240)) << 8) + (((x * 0xff) / 320));
            //printf("X: %d | Y: %d | Color: 0x%x\n\r",x, y, color);
            put_pixel(x, y, color);
        }
    }

    while(1){
        msleep(10000);
    }
}


// Deixar isso aqui na parte de baixo de tudo
task_t display_task = {
    main, // Ponto de entrada da tarefa
    1500  // Quantas words são armazenadas no stack
};

