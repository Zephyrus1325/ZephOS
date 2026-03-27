#include "text_render.h"
#include "core/filesystem.h"
#include "file_parser.h"
#include "include/syscall.h"


struct {
    uint16_t num_tables;

} font_data;



void text_init(){
    FILE* font = fopen("courier.ttf", "r");

    printf("Inicio: %d\n\r", font->fpos);

    uint32_t version = read_uint32(font); // sfntVersion
    font_data.num_tables = read_uint16(font); // NumTables

    printf("Tables: %x | Version: 0x%x | fp: %d\n\r", (uint32_t)font_data.num_tables, (uint32_t)version, font->fpos);



    fclose(font);
}



void draw_char(){

}