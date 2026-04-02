#ifndef FILE_TABLES_H
#define FILE_TABLES_H

#include "stdint.h"

typedef struct {
    uint32_t tag;
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
} __attribute__((packed)) table_data_t;


typedef struct {
    uint16_t num_tables;
    table_data_t* tables;
} font_data_t;

font_data_t font_data;


typedef struct cmap_table_t {
    uint16_t platform_id;
    uint16_t encoding_id;
    uint32_t offset;
} __attribute__((packed)) cmap_table_t;

typedef struct cmap_subtable_t {
    uint16_t format;
    uint16_t length;
    uint16_t seg_count;
    uint16_t* end_code;
    uint16_t* start_code;
    int16_t* id_delta;
    uint16_t* id_range_offset;
    uint16_t* glyph_id_array;
} __attribute__((packed)) cmap_subtable_t;

typedef struct glyph_t {
    uint16_t* end_point_index;
    uint8_t* flags;
    uint8_t* x;
    uint8_t* y;
} __attribute__((packed)) glyph_t;

/*
 *  DADOS DA FONTE:
 *  font_t {
 *      - character mapping
 *      - glyph data
 *      - horizontal data
 * 
 * }
 */

#endif //FILE_TABLES_H
