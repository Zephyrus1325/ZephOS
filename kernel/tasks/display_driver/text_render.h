#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

typedef struct{
    struct cursor{
        uint32_t x;
        uint32_t y;
    };
    
    struct cmap_subtable {
    uint16_t format;
    uint16_t length;
    uint16_t seg_count;
    uint16_t* end_code;
    uint16_t* start_code;
    int16_t* id_delta;
    uint16_t* id_range_offset;
    uint16_t* glyph_id_array;
    } __attribute__((packed));

    struct glyph {
    uint16_t* end_point_index;
    uint8_t* flags;
    uint8_t* x;
    uint8_t* y;
    } __attribute__((packed));

}font_t;


void text_init();

// Adições novas (a implementar)
void get_font(const char* file_name, font_t font);      // Carrega um arquivo de fonte e guarda seus dados
void set_cursor(uint32_t x, uint32_t y);                // Coloca o cursor no pixel superior esquerdo do texto
void set_font_size(float font_size, font_t font);       // Define o tamanho da fonte (em pixels????)
void draw_text(const char* str, font_t font, uint32_t color);   // Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor



#endif // TEXT_RENDERER_H