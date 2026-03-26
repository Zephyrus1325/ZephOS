#include "include/syscall.h"
#include "core/task.h"
#include "core/fifo.h"
#include "core/filesystem.h"
#include "core/memory.h"
#include "drivers/uart.h"
#include "drivers/interrupts.h"
#include "drivers/timer.h"

typedef void (*putc_func_t)(char);
extern void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args);

/* --- Despachante de Syscalls do Kernel --- */
// Função Atômica (aka: não vão acontecer interrupções dentro dela)
void k_svc_dispatcher(uint32_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    context_t* context = (context_t*)current_task->sp;
    switch (id) {
        case SYS_PUTC:
            k_uart_putc((char)arg1);
            return;

        case SYS_GETCHAR:
            context->r0 = (int32_t)k_fifo_get();
            return;

        case SYS_PRINTF:
            // arg1: fmt, arg2: va_list pointer
            k_vprintf_internal(k_uart_putc, (const char*)arg1, *(va_list*)arg2);
            //k_uart_print_no_interrupt((char*)arg1);
            return;

        case SYS_GETPID:
            context->r0 = (int32_t)k_get_current_task_id();
            return;

        case SYS_YIELD:
            scheduler();
            return;

        case SYS_SLEEP:
            current_task->sleep_ticks = arg1;
            current_task->state = TASK_SLEEPING;
            scheduler(); 
            return;
        
        case SYS_MILLIS:
            context->r0 = system_millis - current_task->start_time;
            return;

        case SYS_SPAWN:
            // arg1: func_ptr, arg2: stack_size
            context->r0 = k_task_create((void (*)(void))arg1, arg2);
            return;

        /* --- Syscalls de Arquivo (ANSI Style) --- */

        case SYS_FOPEN:
            // arg1: filename, arg2: mode
            context->r0 = (uint32_t) k_fopen((const char*)arg1, (const char*)arg2);
            return;
        case SYS_FCLOSE:
            // arg1: FILE* fp
            context->r0 =  (int32_t)k_fclose((FILE*)arg1);
            return;

        case SYS_FREAD:
            // arg1 = ptr
            // arg2 = size
            // arg3 = nmemb
            // ONDE ESTÁ O FILE*? 
            // Se usarmos 4 args no assembly, podemos passar o FILE* via pilha ou R4.
            // Para seguir o padrão ANSI perfeitamente, o fread precisaria de 5 args (ID + 4).
            // SOLUÇÃO: Passamos (size * nmemb) consolidado no arg2 e o FILE* no arg3.
            context->r0 = (int32_t)k_fread((void*)arg1, 1, (size_t)arg2, (FILE*)arg3);
            return;

        case SYS_REMOVE:
            // arg1: filename
            context->r0 = (int32_t)k_remove((const char*)arg1);
            return;
        /* --- Memória --- */

        case SYS_MALLOC:
            context->r0 = (int32_t)k_malloc((size_t)arg1);
            return;

        case SYS_FREE:
            k_free((void*)arg1);
            return;

        case SYS_EXIT:
            k_uart_print_no_interrupt("[Kernel] Task exited.\n\r");
            current_task->state = TASK_TERMINATED;
            scheduler();
            return;

        default:
            k_uart_printf_no_interrupt("[Kernel][TASK %d] Error: Unknown Syscall ID: %d\n\r", current_task->id, id);
            return;
    }
}