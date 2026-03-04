#include <include/syscall.h>
#include "task.h"
#include <stdint.h>
#include "core/task.h"
#include "drivers/uart.h"
#include "core/fifo.h"
#include "core/filesystem.h"
#include "drivers/interrupts.h"

extern void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args);

/*
    Esta função é chamada pelo Assembly (svc_handler)

    @param R0 id
    @param R1 arg1
    @param R2 arg2
    @param R3 arg3

    @return Retorno da syscall, ou código de sucesso/falha
*/
int32_t k_svc_dispatcher(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    switch (id) {
        
        case SYS_PRINT:
            /* arg1 contém o ponteiro para a string na memória da tarefa */
            k_uart_print((char *)arg1);
            return 0; // Sucesso

        case SYS_PUTC:
            /* arg1 contém o ponteiro para a string na memória da tarefa */
            k_uart_putc((char) arg1);
            return 0; // Sucesso

        case SYS_PRINTF:
            k_disable_interrupts();
            k_vprintf_internal(k_uart_putc, (const char*)arg1, *(va_list*)arg2);
            k_enable_interrupts();
            return 0;
        
        case SYS_GETCHAR:
            // Tenta pegar o caractere da FIFO
            // Se não tiver nada, retorna -1 para o usuário
            // O programa de usuário pode então decidir se faz um Yield ou espera
            return (int32_t) k_fifo_get();
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

        case SYS_OPEN_FILE:
            return (int32_t) k_fs_open((const char*)arg1, (file_t*)arg2);

        case SYS_CLOSE_FILE:
            return (int32_t) k_fs_close((file_t*)arg2);

        default:
            /* Caso a tarefa peça algo que não existe */
            k_uart_print("[Kernel] Error: Unknown Syscall ID: 0x");
            k_uart_print_hex(id);
            k_uart_print("\n\r");
            return -1;
    }
}