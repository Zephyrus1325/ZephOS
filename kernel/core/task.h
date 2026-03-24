#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stddef.h>

//#define STACK_SIZE 1024 // Tamanho padrão do stack para cada task
#define MAX_TASKS 256   // Quantidade maxima de tasks

// O contexto de uma tarefa ARMv7 (registradores principais)
typedef struct {
    uint32_t sp;   // Stack Pointer
    uint32_t lr;   // Link Register
    uint32_t cpsr; // Status do processador
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;     // Registradores de uso geral
    uint32_t pc;   // Program Counter (onde a tarefa parou)
} context_t;

// Status da tarefa
typedef enum {
    TASK_READY,
    TASK_SLEEPING,
    TASK_WAITING_IO,
    TASK_TERMINATED
} task_state_t;

// task control block (Apenas no kernel sabe dele)
typedef struct tcb_t{
    uint32_t id;            // Task ID, ou Process ID
    task_state_t state;     // Estado da tarefa
    uint32_t sleep_ticks;   // Quantos ticks faltam
    uint32_t* sp;
    uint32_t* stack_base;   // Endereço Base da Task
} tcb_t;

extern tcb_t task_table[MAX_TASKS];
extern uint32_t task_count;
extern tcb_t* current_task;

void k_task_init(tcb_t *tcb, uint32_t id, void (*task_func)(void), uint32_t* stack_mem, uint32_t stack_size);
void scheduler();
int32_t k_task_create(void (*entry_point)(void), size_t stack_size);
int32_t k_task_create_no_interrupt(void (*entry_point)(void), size_t stack_size);
uint32_t k_get_current_task_id(void);
void k_idle_task_init(void);
void k_tick_handler(void);


#endif