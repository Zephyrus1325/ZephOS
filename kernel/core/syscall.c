#include <include/syscall.h>
#include "task.h"
#include <stdint.h>
#include "core/task.h"
#include "drivers/uart.h"
#include "core/fifo.h"

/**
 * k_svc_dispatcher
 * Esta função é chamada pelo Assembly (svc_handler)
 * R0 -> id
 * R1 -> arg1
 * R2 -> arg2
 * R3 -> arg3
 */

int32_t k_svc_dispatcher(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    switch (id) {
        
        case SYS_WRITE:
            /* arg1 contém o ponteiro para a string na memória da tarefa */
            k_uart_print((char *)arg1);
            return 0; // Sucesso

        case SYS_PUTC:
            /* arg1 contém o ponteiro para a string na memória da tarefa */
            k_uart_putc((char) arg1);
            return 0; // Sucesso

        case SYS_GETCHAR:
            // Tenta pegar o caractere da FIFO
            int c = k_fifo_get();
            // Se não tiver nada, retorna -1 para o usuário
            // O programa de usuário pode então decidir se faz um Yield ou espera
            return (int32_t)c;
        case SYS_GETPID:
            /* Retorna o ID da tarefa atual para ser lido no R0 do usuário */
            return (int32_t)k_get_current_task_id();

        case SYS_YIELD:
            /* A tarefa voluntariamente cede a CPU */
            scheduler();
            return 0;   // Scheduler foi chamado, alterar task

        case SYS_SLEEP:
            // arg1 é o tempo em milissegundos (assumindo 1 tick = 1ms)
            current_task->sleep_ticks = arg1;
            current_task->state = TASK_SLEEPING;

            // Força uma troca de contexto imediata para não gastar o resto do time-slice
            scheduler(); 
            return 0;   // Scheduler foi chamado, alterar task

        case SYS_SPAWN:
            return k_task_create((void (*)(void))arg1, arg2);

        default:
            /* Caso a tarefa peça algo que não existe */
            k_uart_print("Kernel Error: Unknown Syscall ID: 0x");
            k_uart_print_hex(id);
            k_uart_print("\n\r");
            return -1;
    }
}