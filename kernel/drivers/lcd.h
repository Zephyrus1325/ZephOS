#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void k_setup_lcd();
void put_pixel(int x, int y, uint32_t color);
void clear_screen(uint32_t color);

#endif // LCD_H