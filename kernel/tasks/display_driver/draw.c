#include "draw.h"
#include "drivers/lcd.h"

void draw_line(point_t start, point_t end, uint32_t color) {
    int dx = (end.x > start.x) ? (end.x - start.x) : (start.x - end.x);
    int dy = (end.y > start.y) ? (end.y - start.y) : (start.y - end.y);
    
    int sx = (start.x < end.x) ? 1 : -1;
    int sy = (start.y < end.y) ? 1 : -1;
    
    int err = dx - dy;
    int x = start.x;
    int y = start.y;

    while (1) {
        // Usa a sua função de pixel que já lida com os limites do framebuffer
        put_pixel(x, y, color);

        if (x == end.x && y == end.y) break;

        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void draw_bezier(point_t* points, uint32_t quality, uint32_t color) {
    if (quality < 1) quality = 1;

    point_t last_point = points[0];
    
    // t varia de 0 a 1.0
    for (uint32_t i = 1; i <= quality; i++) {
        float t = (float)i / (float)quality;
        float inv_t = 1.0f - t;

        // Fórmula: B(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
        float c0 = inv_t * inv_t;
        float c1 = 2.0f * inv_t * t;
        float c2 = t * t;

        point_t next_point;
        next_point.x = (uint16_t)(c0 * points[0].x + c1 * points[1].x + c2 * points[2].x);
        next_point.y = (uint16_t)(c0 * points[0].y + c1 * points[1].y + c2 * points[2].y);

        // Conecta o ponto anterior ao novo ponto calculado
        draw_line(last_point, next_point, color);
        
        last_point = next_point;
    }
}

void draw_rect(uint32_t x0, uint32_t y0, uint32_t width, uint32_t height, uint32_t color){
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            put_pixel(x0 + x, y0 + y, color);
        }
    }
}