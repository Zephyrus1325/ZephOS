#include "file_parser.h"
#include "include/syscall.h"

int8_t read_int8(FILE* file){
    int8_t out;
    fread(&out, sizeof(out), 1, file);
    return out;
}

uint8_t read_uint8(FILE* file){
    uint8_t out;
    fread(&out, sizeof(out), 1, file);
    return out;
}

int16_t read_int16(FILE* file){
    int16_t out;
    fread(&out, sizeof(out), 1, file);
    return ((out >> 8) & (0xff << 0)) + ((out << 8) & (0xff << 8));
}

uint16_t read_uint16(FILE* file){
    uint16_t out;
    fread(&out, sizeof(out), 1, file);
    return ((out >> 8) & (0xff << 0)) + ((out << 8) & (0xff << 8));
}

int32_t read_int32(FILE* file){
    int32_t out;
    fread(&out, sizeof(out), 1, file);
    return ((out >> 24) & (0xff << 0)) + ((out >> 8) & (0xff << 8)) + ((out << 8) & (0xff << 16)) + ((out << 24) & (0xff << 24));
}

uint32_t read_uint32(FILE* file){
    uint32_t out;
    fread(&out, sizeof(out), 1, file);
    return ((out >> 24) & (0xff << 0)) + ((out >> 8) & (0xff << 8)) + ((out << 8) & (0xff << 16)) + ((out << 24) & (0xff << 24));
}
