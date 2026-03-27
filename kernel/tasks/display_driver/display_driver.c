#include "display_driver.h"
#include "include/syscall.h"

#include "drivers/lcd.h"
#include "text_render.h"

/*
 *  Task Responsável por criar todo os gráficos básicos do sistema
 *  Boa parte das coisas do so devem ser um service para esta task
 */

static void main(){
    
    text_init();
    clear_screen(0);
    
    uint32_t color = 0;
    for(int y = 0; y < 480; y++){
        for(int x = 0; x < 640; x++){
            color = ((x * 255) / 640 << 16) + (((y * 255) / 480)); // y % 50 < 25 ? 0xFF : 0;//
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

