#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "core/filesystem.h"

int8_t read_int8(uint8_t* file, uint32_t* address);
uint8_t read_uint8(uint8_t* file, uint32_t* address);
int16_t read_int16(uint8_t* file, uint32_t* address);
uint16_t read_uint16(uint8_t* file, uint32_t* address);
int32_t read_int32(uint8_t* file, uint32_t* address);
uint32_t read_uint32(uint8_t* file, uint32_t* address);

#endif