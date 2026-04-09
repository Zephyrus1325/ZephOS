#include "text_render.h"
#include "core/filesystem.h"
#include "file_parser.h"
#include "include/syscall.h"
#include "file_tables.h"

#include "draw.h"

/*

COISAS PARA FAZER EM RELAÇÃO AS FONTES:

CARREGAMENTO INICIAL:
- struct font_t vai possuir todas as propriedades cruciais da fonte, a fim de carregar tudo rapidamente e de maneira eficiente para o sistema.
DEVE SER CARREGADO:
- TABELAS BASICAS:
    'cmap' 	character to glyph mapping
    'glyf' 	glyph data
    'head' 	font header
    'hhea' 	horizontal header
    'hmtx' 	horizontal metrics
    'loca' 	index to location
    'maxp' 	maximum profile

USO:
- USAR A TABELA CACHEADA


DESCARREGAMENTO
- Desaloca Tudo LMAO


*/

void text_init(){

}

// Carrega um arquivo de fonte e guarda seus dados
void get_font(const char* file_name, font_t font){
    FILE* font_file = fopen("courier.ttf", "r");
    fskip(4, font_file);    // sntf_version
    uint16_t num_tables = read_uint16(font_file);
    fskip(6, font_file);    // search_range | entry_selector | range_shift
}

// Coloca o cursor no pixel superior esquerdo do texto
void set_cursor(uint32_t x, uint32_t y){
    
}

// Define o tamanho da fonte (em pixels????)
void set_font_size(float font_size, font_t font){

}   

// Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor
void draw_text(const char* str, font_t font, uint32_t color){

}