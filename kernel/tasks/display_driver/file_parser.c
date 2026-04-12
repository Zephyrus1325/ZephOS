#include "file_parser.h"
#include "include/syscall.h"

int8_t read_int8(uint8_t* file, uint32_t* address){
    int8_t out = file[*address];
    *address += 1;
    return out;
}

uint8_t read_uint8(uint8_t* file, uint32_t* address){
    uint8_t out = file[*address];
    *address += 1;
    return out;
}

int16_t read_int16(uint8_t* file, uint32_t* address){
    int16_t out = (file[*address] << 8) + file[(*address) + 1];
    *address += 2;
    return out;
}

uint16_t read_uint16(uint8_t* file, uint32_t* address){
    uint16_t out = (file[*address] << 8) + file[(*address) + 1];
    *address += 2;
    return out;
}

int32_t read_int32(uint8_t* file, uint32_t* address){
    int32_t out = (file[*address] << 24) + (file[(*address) + 1] << 16) + (file[(*address) + 2] << 8) + file[(*address) + 3];
    *address += 4;
    return out;
}

uint32_t read_uint32(uint8_t* file, uint32_t* address){
    uint32_t out = (file[*address] << 24) + (file[(*address) + 1] << 16) + (file[(*address) + 2] << 8) + file[(*address) + 3];
    *address += 4;
    return out;
}