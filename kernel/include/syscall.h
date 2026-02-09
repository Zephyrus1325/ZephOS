#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_WRITE    1
#define SYS_GETCHAR  2
#define SYS_GETPID   3
#define SYS_YIELD    4
#define SYS_SLEEP    5
#define SYS_SPAWN    6
#define SYS_CREATE_DIRECTORY // char* directory
#define SYS_REMOVE_DIRECTORY // char* directory
#define SYS_RENAME_DIRECTORY // char* directory, char* name
#define SYS_OPEN_FILE   // uint8_t* fileptr, char* file, char read_write
#define SYS_CLOSE_FILE  // uint8_t* fileptr
#define SYS_CREATE_FILE // char* file, char* name 
#define SYS_REMOVE_FILE // char* file
#define SYS_RENAME_FILE // char* file, char* name
#define SYS_RUN_FILE    // char* file
#define SYS_GET_ACTIVE_PROCESSES 
#define SYS_KILL_PROCESS    // uint32_t PID
#define SYS_MALLOC
#define SYS_FREE
#define SYS_REALLOC
#define SYS_EXIT

#include <stdint.h>

int32_t sys_spawn(void (*func)(void));
void sys_msleep(uint32_t ms);
void sys_write(char *msg);
int32_t sys_getc();

#endif