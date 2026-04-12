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
    FILE* font_file = fopen(file_name, "r");
    fskip(4, font_file);    // sntf_version
    uint16_t num_tables = read_uint16(font_file);
    fskip(6, font_file);    // search_range | entry_selector | range_shift
    
    // Aloca memória para pesquisar as tabelas depois
    uint32_t* table_id = (uint32_t*)malloc(sizeof(uint32_t)*num_tables);
    uint32_t* table_offset = (uint32_t*)malloc(sizeof(uint32_t)*num_tables);

    // Pega as tabelas do arquivo
    for(int i = 0; i < num_tables; i++){
        table_id[i] = read_uint32(font_file);
        fskip(4, font_file);
        table_offset[i] = read_uint32(font_file);
        fskip(4, font_file);
    }

    printf("head\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "head" table
    uint16_t id =  get_table_id("head", table_id, num_tables);
    fsetp(table_offset[id] + 16, font_file);

    font->head.flags = read_uint16(font_file);
    font->head.units_per_em = read_uint16(font_file);
    fskip(8, font_file);
    font->head.x_min = read_int16(font_file);
    font->head.y_min = read_int16(font_file);
    font->head.x_max = read_int16(font_file);
    font->head.y_max = read_int16(font_file);
    fskip(6, font_file);
    font->head.units_per_em = read_int16(font_file);

    printf("maxp\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "maxp" table
    id =  get_table_id("maxp", table_id, num_tables);
    fsetp(table_offset[id] + 4, font_file);
    font->maxp.num_glyphs = read_uint16(font_file);
    font->maxp.max_points = read_uint16(font_file);
    font->maxp.max_countours = read_uint16(font_file);
    font->maxp.max_component_points = read_uint16(font_file);
    font->maxp.max_component_contours = read_uint16(font_file);

    printf("cmap\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "cmap"
    id =  get_table_id("cmap", table_id, num_tables);
    fsetp(table_offset[id] + 2, font_file);

    uint16_t num_cmap_subtables = read_uint16(font_file);

    uint32_t cmap_table_offset;
    
    for(int i = 0; i < num_cmap_subtables; i++){
        uint16_t platform_id = read_uint16(font_file);
        
        if(platform_id == 3){  // Tentar usar sempre a versão unicode
            uint16_t platform_specific_id = read_uint16(font_file);
            cmap_table_offset = read_uint32(font_file);
            break;
        } else {
            fskip(6, font_file);
        }
    }

      
    fsetp(table_offset[id] + cmap_table_offset, font_file);
    
    // Carregar tabela do formato 4
    uint16_t format = read_uint16(font_file);
    if(format != 4){
        printf("INVALID TTF FORMAT: %d\n\r", format);
        return;
    }
    fskip(4, font_file);
    font->cmap.seg_countX2 = read_uint16(font_file);
    fskip(6, font_file);

    // Aloca os recursos necessários
    font->cmap.end_code =        (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.start_code =      (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.id_delta =        (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.id_range_offset = (uint16_t*) malloc(font->cmap.seg_countX2);
    font->cmap.glyph_index =     (uint16_t*) malloc(font->cmap.seg_countX2);
    
    
    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.end_code[i] = read_uint16(font_file);
    }
    fskip(2, font_file);    // Padding

    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.start_code[i] = read_uint16(font_file);
    }

    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.id_delta[i] = read_uint16(font_file);
    }
    
    for(int i = 0; i < (font->cmap.seg_countX2/2); i++){
        font->cmap.id_range_offset[i] = read_uint16(font_file);
    }

    for(int i = 0; i < (font->maxp.num_glyphs); i++){
        font->cmap.glyph_index[i] = read_uint16(font_file);
    }

    printf("hhea\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "hhea"
    id =  get_table_id("hhea", table_id, num_tables);
    fsetp(table_offset[id] + 34, font_file);

    font->hhea.num_hor_metrics = read_uint16(font_file);

    printf("hmtx\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "hmtx"
    id =  get_table_id("hmtx", table_id, num_tables);
    fsetp(table_offset[id], font_file);

    for(int i = 0; i < font->hhea.num_hor_metrics; i++){
        font->hmtx.h_metrics[i].advance_width = read_uint16(font_file); 
        font->hmtx.h_metrics[i].left_side_bearing = read_int16(font_file); 
    }

    for(int i = 0; i < (font->maxp.num_glyphs - font->hhea.num_hor_metrics); i++){
        font->hmtx.left_side_bearing[i] = read_int16(font_file); 
    }

    printf("loca\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "loca"
    id =  get_table_id("loca", table_id, num_tables);
    fsetp(table_offset[id], font_file);

    font->loca.offsets = (uint32_t*) malloc(font->maxp.num_glyphs * sizeof(uint32_t));
    for(int i = 0; i < font->maxp.num_glyphs + 1; i++){
        if(font->head.index_to_loc_format){ 
            font->loca.offsets[i] = read_uint16(font_file) * 2; // short offsets
        } else {    
            font->loca.offsets[i] = read_uint32(font_file);     // long offsets
        }
    }       
    
    printf("glyf\n\r");
    //--------------------------------------------------------------------//
    // Pega os dados do "glyf"
    id =  get_table_id("glyf", table_id, num_tables);
    
    font->glyf = malloc(font->maxp.num_glyphs * sizeof(glyf_t));

    printf("num glyps: %d\n\r",font->maxp.num_glyphs);

    for(int glyph_id = 0; glyph_id < font->maxp.num_glyphs; glyph_id++){
        printf("glif id: %d\n\r", glyph_id);
        fsetp(table_offset[id] + font->loca.offsets[glyph_id], font_file);

        font->glyf[glyph_id].num_contours = read_int16(font_file);
        font->glyf[glyph_id].x_min = read_int16(font_file);
        font->glyf[glyph_id].y_min = read_int16(font_file);
        font->glyf[glyph_id].x_max = read_int16(font_file);
        font->glyf[glyph_id].y_max = read_int16(font_file);
        if(font->glyf[glyph_id].num_contours >= 0){    // Glifos normais
            
            font->glyf->simple.end_points = (uint16_t*) malloc(sizeof(uint16_t) * font->glyf[glyph_id].num_contours);
            font->glyf->simple.x = (uint16_t*) malloc(sizeof(uint16_t) * font->glyf[glyph_id].num_contours);
            font->glyf->simple.y = (uint16_t*) malloc(sizeof(uint16_t) * font->glyf[glyph_id].num_contours);

            for(int n = 0; n < font->glyf[glyph_id].num_contours; n++){
                font->glyf->simple.end_points[n] = read_uint16(font_file);
            }
            
            fskip(read_uint16(font_file), font_file);
            
            for(int n = 0; n < font->glyf[glyph_id].num_contours; n++){
                uint8_t flag = read_uint8(font_file);
                
                if(flag & (1 << 3)){        // Repeat byte ativado
                    uint8_t repeat = read_uint8(font_file);
                    for(int r = 0; r <= repeat; r++){               // <==== Ponto fraco pode ser aqui 
                        font->glyf->simple.flags[n++] = flag;
                    }
                } 
                font->glyf->simple.end_points[n] = flag;
            }

            // pontos x
            for(int n = 0; n < font->glyf[glyph_id].num_contours; n++){
                if(font->glyf[glyph_id].simple.flags[n] & (1 << 1)){ // short vector
                    font->glyf[glyph_id].simple.x[n] = read_uint8(font_file);
                } else {            // l o n g   v e c t o r
                    if(font->glyf[glyph_id].simple.flags[n] & (1 << 4)){  // Mesmo x de antes?
                        font->glyf[glyph_id].simple.x[n] = 0;
                    } else {
                        font->glyf[glyph_id].simple.x[n] = read_int16(font_file);
                    }
                    
                }
            }

            // pontos y
            for(int n = 0; n < font->glyf[glyph_id].num_contours; n++){
                if(font->glyf[glyph_id].simple.flags[n] & (1 << 2)){ // short vector
                    font->glyf[glyph_id].simple.x[n] = read_uint8(font_file);
                } else {            // l o n g   v e c t o r
                    if(font->glyf[glyph_id].simple.flags[n] & (1 << 5)){  // Mesmo x de antes?
                        font->glyf[glyph_id].simple.x[n] = 0;
                    } else {
                        font->glyf[glyph_id].simple.x[n] = read_int16(font_file);
                    }
                    
                }
            }

        } else {                                // Glifos compostos
            // Preguiça de fazer ._.
        }
    }

}

// Coloca o cursor no pixel superior esquerdo do texto
void set_cursor(uint32_t x, uint32_t y){
    
}

// Define o tamanho da fonte (em pixels????)
void set_font_size(float font_size, font_t* font){

}   

// Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor
void draw_char(const char c, font_t* font, uint32_t color){
    // Calcula o offset do caracter
    
    uint16_t font_offset;
    for(int i = 0; i < font->cmap.seg_countX2/2; i++){
        if(font->cmap.start_code[i] < c && font->cmap.end_code[i] < c){
            font_offset = c - font->cmap.id_delta[i];
        }
    }
    
}

// Desenha o texto na tela, utilizando uma fonte, na posição dada pelo cursor
void draw_text(const char* str, font_t* font, uint32_t color){
    printf("desdem\n\r");
    draw_char(str[0], font, color);
}