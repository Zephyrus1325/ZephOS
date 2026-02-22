// user_lib.c ou unistd.c
#include "syscall.h"
#include "drivers/uart.h"

int32_t syscall(int32_t num, uint32_t a1, uint32_t a2, uint32_t a3) {
    int32_t ret;
    asm volatile (
        "mov r0, %[n]\n"    // Move o ID para r0
        "mov r1, %[arg1]\n" // Move argumentos
        "mov r2, %[arg2]\n"
        "mov r3, %[arg3]\n"
        "svc #0\n"          // Chama o Kernel
        "mov %[r], r0\n"    // Pega o retorno do Kernel
        : [r] "=r" (ret)
        : [n] "r" (num), [arg1] "r" (a1), [arg2] "r" (a2), [arg3] "r" (a3)
        : "r0", "r1", "r2", "r3", "memory"
    );
    return ret;
}

int32_t sys_spawn(void (*func)(void)) {
    return syscall(SYS_SPAWN, (uint32_t)func, 0, 0);
}

void sys_msleep(uint32_t ms) {
    syscall(SYS_SLEEP, ms, 0, 0);
}


void sys_write(char *msg) {
    syscall(SYS_WRITE, (uint32_t)msg, 0, 0); 
}

void sys_putc(char c) {
    syscall(SYS_PUTC, (uint32_t)c, 0, 0); 
}

void sys_printf(char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    
    k_vprintf_internal(sys_putc, fmt, args);
    
    va_end(args);
}

int32_t sys_getc() {
    return syscall(SYS_GETCHAR, 0, 0, 0);
}