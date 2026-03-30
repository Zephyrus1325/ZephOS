#include "text_render.h"
#include "core/filesystem.h"
#include "file_parser.h"
#include "include/syscall.h"
#include "file_tables.h"

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
    int16_t xMin = read_int16(font);
    int16_t yMin = read_int16(font);
    int16_t xMax = read_int16(font);
    int16_t yMax = read_int16(font);

    fskip(6, font);

    int16_t index_to_loc_format = read_int16(font);

    //printf("units per em: %d | xMin: %d | xMax: %d | yMin: %d | yMax: %d | format: %d\n\r", (uint32_t)units_per_em, (int32_t)xMin, (int32_t)xMax, (int32_t)yMin, (int32_t)yMax, (int32_t)index_to_loc_format);

    // Leitura do "cmap"
    fsetp(font_data.tables[id_cmap].offset, font);

    printf("fp: 0x%x\n\r", font->fpos);
    
    uint16_t v = read_int16(font);
    printf("fp: 0x%x | V = 0x%x\n\r", font->fpos, v);
    
    uint16_t num_cmap_tables = read_uint16(font);
    printf("fp: 0x%x | numtable = 0x%x\n\r", font->fpos, num_cmap_tables);
    
    //cmap_table_t* cmap_tables = (cmap_table_t*)malloc(sizeof(cmap_table_t) * num_cmap_tables);

    printf("Version: %x | num tables: %x\n\r", (uint32_t)v, (uint32_t)num_cmap_tables);

    //for(int i = 0; i < num_cmap_tables; i++){
    //    cmap_tables[i].platform_id = read_uint16(font);
    //    cmap_tables[i].encoding_id = read_uint16(font);
    //    cmap_tables[i].offset = read_uint32(font);
    //    //printf("PlatformID: %d | encodingID: %d | offset: 0x%x\n\r", cmap_tables[i].platform_id,cmap_tables[i].encoding_id,cmap_tables[i].offset);
    //}

    fclose(font);
}



void draw_char(){

}