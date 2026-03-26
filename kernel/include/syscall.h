#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "core/filesystem.h"

enum syscalls {
    SYS_PUTC = 0,
    SYS_GETCHAR,
    SYS_PRINTF,
    SYS_GETPID,
    SYS_YIELD,
    SYS_SLEEP,
    SYS_MILLIS,
    SYS_SPAWN,
    SYS_FOPEN,     // uint32_t ID: 7
    SYS_FCLOSE,
    SYS_FREAD,
    SYS_FWRITE,    // Adicionado para completude
    SYS_REMOVE,
    SYS_MALLOC,
    SYS_FREE,
    SYS_EXIT,
    SYS_KILL_PROCESS
};

/* --- Interface ANSI C (stdio.h / stdlib.h style) --- */

// Gerenciamento de Arquivos
FILE*   fopen(const char* filename, const char* mode);
int     fclose(FILE* stream);
size_t  fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
int     fgetc(FILE* stream);
char*   fgets(char* s, int n, FILE* stream);
int     remove(const char* filename);

// I/O de Console
int     printf(const char* format, ...);
int     putchar(int character);
int     getchar(void);

// Processos e Memória
void*   malloc(size_t size);
void    free(void* ptr);
void    exit(int status);
int32_t spawn(void (*func)(void), size_t stack_size);
void    msleep(uint32_t ms);
uint32_t millis();

#endif