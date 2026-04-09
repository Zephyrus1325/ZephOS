#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <stdint.h>

typedef struct {
    int16_t num_contours;
    float x_min;
    float y_min;
    float x_max;
    float y_max;

    union {
        struct {
            uint16_t* end_points;
            uint8_t* flags;
            void* x;
            void* y;
        } simple;
        
        struct {
            uint16_t flags;
            uint16_t glyph_index;
            int16_t arg1;
            int16_t arg2;
        } compound;
    };
} glyf_t;
struct longHorMetric_t{
    uint16_t advance_width;
    int16_t left_side_bearing;
}longHorMetric;


typedef struct font_t{

    struct meta_t {
        uint32_t color;
        float size;
        struct cursor{
            uint32_t x;
            uint32_t y;
        } cursor;
    } meta;
    
    struct cmap_t {
        uint16_t seg_countX2;
        uint16_t* end_code;
        uint16_t* start_code;
        uint16_t* id_delta;
        uint16_t* id_range_offset;
        uint16_t* glyph_index;
    } cmap;  

    glyf_t* glyf;
    
    struct head_t{
        uint16_t flags;
        uint16_t units_per_em;
        int16_t x_min;
        int16_t y_min;
        int16_t x_max;
        int16_t y_max;
        int16_t index_to_loc_format;
    } head;


    struct hhea_t{
        uint16_t num_hor_metrics
    } hhea;

    struct htmx_t {
        struct longHorMetric* h_metrics;
        int16_t* left_side_bearing;
    } hmtx;

    struct loca_t{
        uint32_t* offsets;
    } loca;

    struct  maxp_t{
        uint16_t num_glyphs;
        uint16_t max_points;
        uint16_t max_countours;
        uint16_t max_component_points;
        uint16_t max_component_contours;
    } maxp;

} font_t;


void text_init();

// Adições novas (a implementar)
void get_font(const char* file_name, font_t font);      // Carrega um arquivo de fonte e guarda seus dados
void set_cursor(uint32_t x, uint32_t y);                // Coloca o cursor no pixel superior esquerdo do texto
void set_font_size(float font_size, font_t font);       // Define o tamanho da fonte (em pixels????)
void draw_text(const char* str, font_t font, uint32_t color);   // Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor



#endif // TEXT_RENDERER_H