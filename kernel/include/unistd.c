// user_lib.c ou unistd.c
#include "stdint.h"
#include "include/syscall.h"
#include "drivers/uart.h"

static inline int32_t _syscall(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    register uint32_t r0 __asm__("r0") = id;
    register uint32_t r1 __asm__("r1") = arg1;
    register uint32_t r2 __asm__("r2") = arg2;
    register uint32_t r3 __asm__("r3") = arg3;
    __asm__ volatile("svc 0" : "=r"(r0) : "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory");
    return (int32_t)r0;
}

FILE* fopen(const char* filename, const char* mode) {
    // Retorna o ponteiro FILE* alocado pelo Kernel (via malloc no kernel space)
    return (FILE*)_syscall(SYS_FOPEN, (uint32_t)filename, (uint32_t)mode, 0);
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    // Consolidamos o tamanho total para caber nos 3 argumentos da syscall
    // arg1: buffer, arg2: total_bytes, arg3: ponteiro do arquivo
    uint32_t total = size * nmemb;
    return (size_t)_syscall(SYS_FREAD, (uint32_t)ptr, total, (uint32_t)stream);
}

int printf(const char* format, ...) {
    //va_list args;
    //va_start(args, format);
    
    //uint32_t args_ptr = (uint32_t)&args;

    int return_val = _syscall(SYS_PRINTF, (uint32_t)format, 0, 0);
    
    //va_end(args);
    return return_val;
}

int putchar(int character) {
    return _syscall(SYS_PUTC, (uint32_t)character, 0, 0);
}

void msleep(uint32_t ms) {
    _syscall(SYS_SLEEP, ms, 0, 0);
}

int fclose(FILE* stream) {
    return _syscall(SYS_FCLOSE, (uint32_t)stream, 0, 0);
}

int getpid(void) {
    return _syscall(SYS_GETPID, 0, 0, 0);
}