#include "text_render.h"
#include "core/filesystem.h"
#include "file_parser.h"
#include "include/syscall.h"
#include "file_tables.h"

#include "draw.h"

// Return ID if successful
// Return -1 if not successful
int32_t get_table_id(font_data_t* font, const char* table_name){
    uint32_t reference_id = table_name[3] + (table_name[2] << 8) + (table_name[1] << 16) + (table_name[0] << 24);
    for(int i = 0; i < font->num_tables; i++){
        if(reference_id == font->tables[i].tag){
            return i;
        }
    }
    return -1;
}



void text_init(){
    FILE* font = fopen("courier.ttf", "r");
    uint32_t version = read_uint32(font); // sfntVersion
    font_data.num_tables = read_uint16(font); // NumTables
    font_data.tables = malloc(sizeof(table_data_t) * font_data.num_tables);
    fskip(6, font);
    for(int i = 0; i < font_data.num_tables; i++){
        font_data.tables[i].tag = read_uint32(font);
        font_data.tables[i].checksum = read_uint32(font);
        font_data.tables[i].offset = read_uint32(font);
        font_data.tables[i].length = read_uint32(font);

        //printf("Table: %c%c%c%c | offset: 0x%x | length: %d\n\r", (char)(font_data.tables[i].tag >> 24) & 0xFF, (char)(font_data.tables[i].tag >> 16) & 0xFF, (char)(font_data.tables[i].tag >> 8) & 0xFF, (char)(font_data.tables[i].tag >> 0) & 0xFF, font_data.tables[i].offset, font_data.tables[i].length);
    }

    // Pega os headers relevantes
    int id_head = get_table_id(&font_data, "head");
    int id_cmap = get_table_id(&font_data, "cmap");
    int id_maxp = get_table_id(&font_data, "maxp");
    int id_hhea = get_table_id(&font_data, "hhea");
    int id_hmtx = get_table_id(&font_data, "hmtx");
    int id_loca = get_table_id(&font_data, "loca");
    int id_glyf = get_table_id(&font_data, "glyf");

    // Leitura do "head"
    fsetp(font_data.tables[id_head].offset, font);

    fskip(16, font);
    
    uint16_t head_flags = read_uint16(font);
    uint16_t units_per_em = read_uint16(font);
    fskip(16, font);
    int16_t font_xMin = read_int16(font);
    int16_t font_yMin = read_int16(font);
    int16_t font_xMax = read_int16(font);
    int16_t font_yMax = read_int16(font);

    fskip(6, font);

    int16_t index_to_loc_format = read_int16(font);

    //printf("units per em: %d | xMin: %d | xMax: %d | yMin: %d | yMax: %d | format: %d\n\r", (uint32_t)units_per_em, (int32_t)xMin, (int32_t)xMax, (int32_t)yMin, (int32_t)yMax, (int32_t)index_to_loc_format);

    // Leitura do "cmap"
    fsetp(font_data.tables[id_cmap].offset, font);

    uint16_t v = read_uint16(font);
    uint16_t num_cmap_tables = read_uint16(font);
    
    cmap_table_t cmap_table;

    // Preenche as tabelas
    for(int i = 0; i < num_cmap_tables; i++){
        cmap_table.platform_id = read_uint16(font);
        if(cmap_table.platform_id == 0){        // ID 0 = UNICODE
            cmap_table.encoding_id = read_uint16(font);
            cmap_table.offset = read_uint32(font);
            break;                              // Encontrou a tabela, ignora o resto
        } else {
            fskip(6, font);
        }
    }

    fsetp(font_data.tables[id_cmap].offset + cmap_table.offset, font);     // Vai para a subtabela de fontes
    
    cmap_subtable_t cmap_subtable;
    cmap_subtable.format = read_uint16(font);
    cmap_subtable.length = read_uint16(font);
    fskip(2, font);     // Language
    cmap_subtable.seg_count = read_uint16(font); // 2 * segCount
    fskip(6, font);

    // Aloca os elementos para os ranges
    cmap_subtable.end_code = (uint16_t*) malloc(cmap_subtable.seg_count);
    cmap_subtable.start_code = (uint16_t*) malloc(cmap_subtable.seg_count);
    cmap_subtable.id_delta = (int16_t*) malloc(cmap_subtable.seg_count);

    // Preenche os ranges
    // End code
    for(int i = 0; i < cmap_subtable.seg_count/2; i++){
        cmap_subtable.end_code[i] = read_uint16(font);
    }

    fskip(2, font);     // Padding (0)
    // Start code
    for(int i = 0; i < cmap_subtable.seg_count/2; i++){
        cmap_subtable.start_code[i] = read_uint16(font);
    }

    // Delta
    for(int i = 0; i < cmap_subtable.seg_count/2; i++){
        cmap_subtable.id_delta[i] = read_int16(font);
    }

    fclose(font);
}



void draw_char(uint16_t x, uint16_t y, char c, font_t font){
    
    char c = 'A';

    uint16_t glyph_index = 0;

    for(int i = 0; i < cmap_subtable.seg_count/2; i++){
        if((cmap_subtable.start_code[i] >= c) && (c <= cmap_subtable.end_code[i])){
            glyph_index = c + cmap_subtable.id_delta[i];
            break;
        }
    }

    size_t glyph_offset;
    
    if(index_to_loc_format){
        fsetp(font_data.tables[id_loca].offset + (sizeof(uint32_t) * glyph_index), font);
        glyph_offset = read_uint32(font);
        
    } else {
        fsetp(font_data.tables[id_loca].offset + (sizeof(uint16_t) * glyph_index), font);
        glyph_offset = read_uint16(font);
    }

    // Vai pra tabela que desenha o glifo em si
    fsetp(font_data.tables[id_glyf].offset + glyph_offset, font);
    //fsetp(font_data.tables[id_glyf].offset, font);

    int16_t num_contours = read_int16(font);
    int16_t xMin = read_int16(font);
    int16_t yMin = read_int16(font);
    int16_t xMax = read_int16(font);
    int16_t yMax = read_int16(font);

    //printf("NumCont: %d | xMin: %d | xMax: %d | yMin: %d | yMax: %d\n\r", (int32_t)num_contours, (int32_t)xMin, (int32_t)xMax, (int32_t)yMin, (int32_t)yMax);

    //for(int i = 0; i < num_contours; i++){
    //
    //}
    draw_rect(xMin + 50, yMin + 20, xMax-xMin, yMax-yMin, 0xFF0000);
}