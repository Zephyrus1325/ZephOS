#include "text_render.h"
#include "core/filesystem.h"
#include "file_parser.h"
#include "include/syscall.h"
#include "file_tables.h"
#include "include/syscall.h"

#include "drivers/lcd.h"

#include "draw.h"

/*

COISAS PARA FAZER EM RELAÇÃO AS FONTES:

CARREGAMENTO INICIAL:
- struct font_t vai possuir todas as propriedades cruciais da fonte, a fim de carregar tudo rapidamente e de maneira eficiente para o sistema.
DEVE SER CARREGADO:
- TABELAS BASICAS:
    -'cmap' 	character to glyph mapping
    'glyf' 	glyph data
    -'head' 	font header
    -'hhea' 	horizontal header
    -'hmtx' 	horizontal metrics
    -'loca' 	index to location
    -'maxp' 	maximum profile

USO:
- USAR A TABELA CACHEADA


DESCARREGAMENTO
- Desaloca Tudo LMAO


*/

uint16_t get_table_id(const char* searched_table, uint32_t* tables, uint16_t num_tables){

    uint32_t s_table = (searched_table[0] << 24) + (searched_table[1] << 16) + (searched_table[2] << 8) + (searched_table[3] << 0);
    for(int i = 0; i < num_tables; i++){
        if(s_table == tables[i]){
            return i;
        }
    }
    return -1;
}


// Carrega um arquivo de fonte e guarda seus dados
void get_font(const char* file_name, font_t* font){
    FILE* font_file = fopen(file_name, "r");       // carrega o arquivo
    font->raw_file = malloc(font_file->size);      // Aloca o tamanho do arquivo todo
    fread(font->raw_file, font_file->size, 1, font_file);
    fclose(font_file);

    uint32_t file_ptr = 0;

    file_ptr += 4;    // sntf_version
    uint16_t num_tables = read_uint16(font->raw_file, &file_ptr);
    file_ptr += 6;    // search_range | entry_selector | range_shift
    
    // Aloca memória para pesquisar as tabelas depois
    uint32_t* table_id = (uint32_t*)malloc(sizeof(uint32_t)*num_tables);
    uint32_t* table_offset = (uint32_t*)malloc(sizeof(uint32_t)*num_tables);

    // Pega as tabelas do arquivo
    for(int i = 0; i < num_tables; i++){
        table_id[i] = read_uint32(font->raw_file, &file_ptr);
        file_ptr += 4;
        table_offset[i] = read_uint32(font->raw_file, &file_ptr);
        file_ptr += 4;
    }


    //--------------------------------------------------------------------//
    // Pega os dados do "head" table
    uint16_t id =  get_table_id("head", table_id, num_tables);
    file_ptr = table_offset[id] + 16;

    font->head.flags = read_uint16(font->raw_file, &file_ptr);
    font->head.units_per_em = read_uint16(font->raw_file, &file_ptr);
    file_ptr += 8;
    font->head.x_min = read_int16(font->raw_file, &file_ptr);
    font->head.y_min = read_int16(font->raw_file, &file_ptr);
    font->head.x_max = read_int16(font->raw_file, &file_ptr);
    font->head.y_max = read_int16(font->raw_file, &file_ptr);
    file_ptr += 6;
    font->head.index_to_loc_format= read_int16(font->raw_file, &file_ptr);


    //--------------------------------------------------------------------//
    // Pega os dados do "maxp" table
    id =  get_table_id("maxp", table_id, num_tables);
    file_ptr = table_offset[id] + 4;
    font->maxp.num_glyphs = read_uint16(font->raw_file, &file_ptr);
    font->maxp.max_points = read_uint16(font->raw_file, &file_ptr);
    font->maxp.max_countours = read_uint16(font->raw_file, &file_ptr);
    font->maxp.max_component_points = read_uint16(font->raw_file, &file_ptr);
    font->maxp.max_component_contours = read_uint16(font->raw_file, &file_ptr);


    //--------------------------------------------------------------------//
    // Pega os dados do "cmap"
    id =  get_table_id("cmap", table_id, num_tables);
    file_ptr = table_offset[id] + 2;

    uint16_t num_cmap_subtables = read_uint16(font->raw_file, &file_ptr);

    uint32_t cmap_table_offset;
    
    for(int i = 0; i < num_cmap_subtables; i++){
        uint16_t platform_id = read_uint16(font->raw_file, &file_ptr);
        
        if(platform_id == 3){  // Tentar usar sempre a versão unicode
            uint16_t platform_specific_id = read_uint16(font->raw_file, &file_ptr);
            cmap_table_offset = read_uint32(font->raw_file, &file_ptr);
            break;
        } else {
            file_ptr += 6;
        }
    }

      
    file_ptr = table_offset[id] + cmap_table_offset, font->raw_file;
    
    // Carregar tabela do formato 4
    uint16_t format = read_uint16(font->raw_file, &file_ptr);
    if(format != 4){
        printf("INVALID TTF FORMAT: %d\n\r", format);
        return;
    }
    file_ptr += 4;
    font->cmap.seg_countX2 = read_uint16(font->raw_file, &file_ptr);
    file_ptr += 6;

    // Aloca os recursos necessários
    font->cmap.end_code =        (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.start_code =      (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.id_delta =        (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.id_range_offset = (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.glyph_index =     (uint16_t*) malloc(font->cmap.seg_countX2);

    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.end_code[i] = read_uint16(font->raw_file, &file_ptr);
    }
    file_ptr += 2;    // Padding

    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.start_code[i] = read_uint16(font->raw_file, &file_ptr);
    }

    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.id_delta[i] = read_uint16(font->raw_file, &file_ptr);
    }
    
    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.id_range_offset[i] = read_uint16(font->raw_file, &file_ptr);
    }

    for(int i = 0; i < (font->maxp.num_glyphs); i++){
        font->cmap.glyph_index[i] = read_uint16(font->raw_file, &file_ptr);
    }


    //--------------------------------------------------------------------//
    // Pega os dados do "hhea"
    id =  get_table_id("hhea", table_id, num_tables);
    file_ptr = table_offset[id] + 34;

    font->hhea.num_hor_metrics = read_uint16(font->raw_file, &file_ptr);

    //--------------------------------------------------------------------//
    // Pega os dados do "hmtx"
    id =  get_table_id("hmtx", table_id, num_tables);
    file_ptr = table_offset[id];

    font->hmtx.h_metrics = malloc(sizeof(longHorMetric_t) * font->hhea.num_hor_metrics);
    font->hmtx.left_side_bearing =  malloc(sizeof(uint32_t) * (font->maxp.num_glyphs - font->hhea.num_hor_metrics));

    for(int i = 0; i < font->hhea.num_hor_metrics; i++){
        font->hmtx.h_metrics[i].advance_width = read_uint16(font->raw_file, &file_ptr); 
        font->hmtx.h_metrics[i].left_side_bearing = read_int16(font->raw_file, &file_ptr); 
    }

    for(int i = 0; i < (font->maxp.num_glyphs - font->hhea.num_hor_metrics); i++){
        font->hmtx.left_side_bearing[i] = read_int16(font->raw_file, &file_ptr); 
    }

    //--------------------------------------------------------------------//
    // Pega os dados do "loca"
    id =  get_table_id("loca", table_id, num_tables);
    file_ptr = table_offset[id];

    font->loca.offsets = (uint32_t*) malloc(font->maxp.num_glyphs * sizeof(uint32_t));
    for(int i = 0; i < font->maxp.num_glyphs + 1; i++){
        if(font->head.index_to_loc_format){ 
            font->loca.offsets[i] = read_uint32(font->raw_file, &file_ptr); // short offsets
        } else {    
            font->loca.offsets[i] = read_uint16(font->raw_file, &file_ptr) * 2;     // long offsets
        }
    }       

    // Coloca o offset da tabela de glifos para facilitar a vida :D
    font->glyf.table_offset = table_offset[get_table_id("glyf", table_id, num_tables)];

    font->meta.cursor.x = 10;
    font->meta.cursor.y = 70;

}

// Coloca o cursor no pixel superior esquerdo do texto
void set_cursor(uint32_t x, uint32_t y){
    
}

// Define o tamanho da fonte (em pixels????)
void set_font_size(float font_size, font_t* font){

}   

void render_char(int16_t x0, int16_t y0, int16_t width, int16_t height, uint8_t* glyph_data, uint32_t color){
    for(int16_t y = y0; y < y0 + height; y++){
        for(int16_t x = x0; x < x0 + width; x++){
            // Rodar codigo abaixo para cada pixel dentro do caracter

        }
    }
}


// Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor
void draw_char(const char c, font_t* font, uint32_t color){
    // Calcula o offset do caracter
    
    uint16_t glyph_index = 0;
    for(int i = 0; i < font->cmap.seg_countX2/2; i++){
        if(font->cmap.start_code[i] < c && c < font->cmap.end_code[i] ){ // O caracter é suportado pela fonte?
           
            glyph_index = c + font->cmap.id_delta[i]; // Se sim, pega o offset dele pela tabela
            // TODO: adaptar se o id_range_offset for diferente de 0
            break;                                                      // Sai do for loop
        }                                                               // Se não, desenha o caracter de desconhecido
    }
   
    uint8_t* glyph_data = font->raw_file + font->glyf.table_offset + font->loca.offsets[glyph_index];  // Inicia um array no local da fonte

    uint32_t glyph_reader_index = 0;
    int16_t num_contours = read_int16(glyph_data, &glyph_reader_index);    

    if(num_contours <= 0){
        // Fontes compostas
        // Ai fudeu ne, lidar com essa bomba depois.
        // Desenha nada por enquanto
        glyph_data = font->raw_file + font->loca.offsets[0] + 2; // Null Character
    }
    int16_t x_min = read_int16(glyph_data, &glyph_reader_index);
    int16_t y_min = read_int16(glyph_data, &glyph_reader_index);
    int16_t x_max = read_int16(glyph_data, &glyph_reader_index);
    int16_t y_max = read_int16(glyph_data, &glyph_reader_index);
    
    uint16_t font_size = 64; // 32 px
    
    uint16_t advance_width;
    int16_t left_side_bearing;

    if(glyph_index < font->hhea.num_hor_metrics){
        advance_width = font->hmtx.h_metrics[glyph_index].advance_width;
        left_side_bearing = font->hmtx.h_metrics[glyph_index].left_side_bearing;
    } else {
        advance_width = font->hmtx.h_metrics[font->hhea.num_hor_metrics - 1].advance_width;
        left_side_bearing = font->hmtx.left_side_bearing[glyph_index - font->hhea.num_hor_metrics];
    }

    int16_t draw_x = font->meta.cursor.x + (left_side_bearing * font_size / font->head.units_per_em) + (x_min * font_size / font->head.units_per_em);
    int16_t draw_y = font->meta.cursor.y - ((y_max * font_size) / font->head.units_per_em);
    int16_t width = (x_max-x_min) * font_size/font->head.units_per_em;
    int16_t height = (y_max-y_min) * font_size/font->head.units_per_em;

    draw_rect(draw_x, draw_y, width, height, 0xFFFFFF);
    font->meta.cursor.x += advance_width * font_size / font->head.units_per_em;
    // printf("%d %d", ((x_max - x_min)*font_size)/font->head.units_per_em, ((y_max - y_min)*font_size)/font->head.units_per_em);
}

// Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor
void draw_text(const char* str, font_t* font, uint32_t color){
    for(int i = 0; str[i] != '\0'; i++){
        draw_char(str[i], font, color);
    }
}