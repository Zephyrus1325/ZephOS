#include "drivers/uart.h"
#include "task.h"
#include "include/syscall.h"
#include "core/memory.h"

void task_init(tcb_t *tcb, uint32_t id, void (*task_func)(void), uint32_t* stack_mem, uint32_t stack_size) {
    // Aponta para o fim da stack
    tcb->stack_base = stack_mem;
    uint32_t *s = (uint32_t *)&tcb->stack_base[stack_size];

    // Ordem inversa dos POPs do Assembly:
    *(--s) = (uint32_t)task_func; // PC (carregado pelo pop {pc}^)
    *(--s) = 0x13;                // SPSR (carregado pelo pop {r1})
    
    // R12, R11, ..., R0 (carregados pelo pop {r0-r11, r12})
    for (int i = 0; i < 13; i++) {
        *(--s) = 0; 
    }
    
    *(--s) = 0;                   // LR (carregado pelo pop {lr})

    tcb->sp = (uint32_t)s;
    tcb->id = id;
    tcb->state = TASK_READY;
    tcb->sleep_ticks = 0;
}

// Controle das tasks (scheduler)

extern tcb_t *current_task;
tcb_t idle_task_tcb;

tcb_t task_table[MAX_TASKS];
unsigned int task_count = 0;


void scheduler(void) {
    // Pegamos o índice atual da tarefa para começar a busca a partir dele
    // Note que usamos um int local para a busca para não estragar o índice global prematuramente
    static int current_idx = 0;
    int search_idx = current_idx;

    // Tentamos encontrar uma tarefa READY no máximo 'task_count' vezes
    for (int i = 0; i < task_count; i++) {
        // Incrementa e rotaciona (sem usar %)
        search_idx++;
        if (search_idx >= task_count) {
            search_idx = 0;
        }

        // AQUI ESTÁ A CHAVE: Só aceita se estiver READY
        if (task_table[search_idx].state == TASK_READY) {
            current_idx = search_idx; // Atualiza o índice oficial
            current_task = &task_table[current_idx];
            return; // Encontrou! Sai da função e volta para a tarefa
        }
    }
    // 2. Se o loop terminou e ninguém foi encontrado (todos SLEEPING):
    // Forçamos a Idle Task.
    current_task = &idle_task_tcb;
}

int k_task_create(void (*entry_point)(void), size_t stack_size) {
    if (task_count >= MAX_TASKS) return -1;

    // Aloca memória para a pilha da nova task no HEAP (SRAM 64MB)
    // STACK_SIZE * 4 porque malloc geralmente recebe tamanho em bytes
    uint32_t *new_stack = (uint32_t *)k_malloc(stack_size * sizeof(uint32_t));
    uart_print("STACK: ");
    uart_print_hex((uint32_t) new_stack);
    uart_print("\n");
    if (new_stack == NULL) return -2; // Erro: Out of Memory na SRAM

    tcb_t *new_tcb = &task_table[task_count];
    
    // Inicializa passando o ponteiro do Heap
    task_init(new_tcb, task_count + 1, entry_point, new_stack, stack_size);

    task_count++;
    return new_tcb->id;
}

uint32_t k_get_current_task_id(void) {
    if (current_task != 0) {
        return current_task->id;
    }
    return 0; // Caso não haja tarefa rodando
}

void idle_task_func(){
    while(1) {
        //sys_write("IDLE\n");
        asm volatile("wfi");
    }
}

void k_idle_task_init(void) {
    uint32_t *idle_stack = (uint32_t *)k_malloc(64 * sizeof(uint32_t));
    task_init(&idle_task_tcb, 0, idle_task_func, idle_stack, 64);
    idle_task_tcb.state = TASK_READY;
}

void k_tick_handler(void) {
    for (int i = 0; i < task_count; i++) {
        if (task_table[i].state == TASK_SLEEPING) {
            if (task_table[i].sleep_ticks > 0) {
                task_table[i].sleep_ticks--;
            }
            
            // Se o tempo acabou, acorda a tarefa
            if (task_table[i].sleep_ticks == 0) {
                task_table[i].state = TASK_READY;
            }
        }
    }
}