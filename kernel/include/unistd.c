// user_lib.c ou unistd.c
#include "syscall.h"
#include "drivers/uart.h"

static inline int32_t syscall(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    register uint32_t r0 __asm__("r0") = id;
    register uint32_t r1 __asm__("r1") = arg1;
    register uint32_t r2 __asm__("r2") = arg2;
    register uint32_t r3 __asm__("r3") = arg3;
    __asm__ volatile("svc 0" : "=r"(r0) : "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory");
    return r0;
}

int32_t sys_spawn(void (*func)(void)) {
    return syscall(SYS_SPAWN, (uint32_t)func, 0, 0);
}

void sys_msleep(uint32_t ms) {
    syscall(SYS_SLEEP, ms, 0, 0);
}


void sys_print(char *msg) {
    syscall(SYS_PRINT, (uint32_t)msg, 0, 0); 
}

void sys_putc(char c) {
    syscall(SYS_PUTC, (uint32_t)c, 0, 0); 
}

void sys_printf(char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    
    syscall(SYS_PRINTF, (uint32_t)fmt, (uint32_t)&args, 0);
    
    va_end(args);
}

int32_t sys_getc() {
    return syscall(SYS_GETCHAR, 0, 0, 0);
}

int32_t sys_open(char* f_name, file_t* file){
    return syscall(SYS_OPEN_FILE, (uint32_t)f_name, (uint32_t)file, 0);
}

int32_t sys_close(file_t* f){
    return syscall(SYS_CLOSE_FILE, 0, 0, 0);
}