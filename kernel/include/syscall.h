#ifndef SYSCALL_H
#define SYSCALL_H

enum syscalls{
    SYS_PRINT,   
    SYS_PRINTF,
    SYS_PUTC, 
    SYS_GETCHAR,  
    SYS_GETPID,   
    SYS_YIELD,    
    SYS_SLEEP,    
    SYS_SPAWN,    
    SYS_CREATE_DIRECTORY, // char* directory
    SYS_REMOVE_DIRECTORY, // char* directory
    SYS_RENAME_DIRECTORY, // char* directory, char* name
    SYS_OPEN_FILE,   // uint8_t* fileptr, char* file, char read_write
    SYS_CLOSE_FILE,  // uint8_t* fileptr
    SYS_CREATE_FILE, // char* file, char* name 
    SYS_REMOVE_FILE, // char* file
    SYS_RENAME_FILE, // char* file, char* name
    SYS_RUN_FILE,    // char* file
    SYS_GET_ACTIVE_PROCESSES, 
    SYS_KILL_PROCESS,    // uint32_t PID
    SYS_MALLOC,
    SYS_FREE,
    SYS_REALLOC,
    SYS_EXIT
};

#include <stdint.h>
#include <stdarg.h>
#include "core/filesystem.h"

typedef void (*putc_func_t)(char);
extern void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args);

int32_t sys_spawn(void (*func)(void));
void sys_msleep(uint32_t ms);
void sys_print(char *msg);
void sys_putc(char c);
void sys_printf(char *fmt, ...);
int32_t sys_getc();
int32_t sys_fsopen(char* f_name, file_t* file);
int32_t sys_fsclose(file_t* f);

// Coisas novas:
file_t* sys_fopen(const char* file_name, const char* mode);
int32_t sys_fclose(file_t* file_ptr);
size_t sys_fread(void *ptr, size_t elem_size, size_t num_elem, file_t* file_ptr);
char* sys_fgets(char* str, uint32_t n, file_t* f);
int32_t sys_fgetc(file_t f);
int32_t sys_remove(const char* file_name);

#endif