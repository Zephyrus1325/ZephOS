#include "display_driver.h"
#include "include/syscall.h"

#include "drivers/lcd.h"
#include "text_render.h"

#include "draw.h"

/*
 *  Task Responsável por criar todo os gráficos básicos do sistema
 *  Boa parte das coisas do so devem ser um service para esta task
 */

static void main(){
    
    font_t* console_font = (font_t*) malloc(sizeof(font_t));
    
    get_font("courier.ttf", console_font);
    
    clear_screen(0);

    uint32_t color = 0;
    for(int y = 0; y < 480; y++){
        for(int x = 0; x < 640; x++){
            color = ((x * 255) / 640 << 16) + (((y * 255) / 480)); // y % 50 < 25 ? 0xFF : 0;//
            put_pixel(x, y, color);
        }
    }

    draw_text("Hello World!", console_font, 0xFFFFFF);

    while(1){
        msleep(10000);
    }
}


// Deixar isso aqui na parte de baixo de tudo
task_t display_task = {
    main, // Ponto de entrada da tarefa
    1500  // Quantas words são armazenadas no stack
};

