#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "core/filesystem.h"

int8_t read_int8(FILE* file);
uint8_t read_uint8(FILE* file);
int16_t read_int16(FILE* file);
uint16_t read_uint16(FILE* file);
int32_t read_int32(FILE* file);
uint32_t read_uint32(FILE* file);

#endif