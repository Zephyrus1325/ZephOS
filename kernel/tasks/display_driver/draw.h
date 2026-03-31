#ifndef DRAW_H
#define DRAW_H

#include "stdint.h"

typedef struct {
    uint16_t x;
    uint16_t y;
} point_t;

void draw_line(point_t start, point_t end, uint32_t color);
void draw_bezier(point_t* points, uint32_t quality, uint32_t color);
void draw_rect(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height, uint32_t color);

#endif // DRAW_H