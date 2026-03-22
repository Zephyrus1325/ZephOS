#include "drivers/uart.h"
#include "task.h"
#include "include/syscall.h"
#include "core/memory.h"
#include "drivers/uart.h"

// Variáveis de tarefas

tcb_t* current_task = 0;
tcb_t idle_task_tcb;

tcb_t task_table[MAX_TASKS];
uint32_t task_count = 0;

/*
    Configura e inicia uma nova tarefa no sistema

    @param *tcb Ponteiro para o tcb responsável por armazenar esta tarefa
    @param id ID a ser atrelado a nova tarefa
    @param stack_mem Ponteiro para o stack da tarefa
    @param stack_size Total de palavras a ser alocados para o stack da tarefa

*/
void k_task_init(tcb_t *tcb, uint32_t id, void (*task_func)(void), uint32_t* stack_mem, uint32_t stack_size) {
    // Preenche o TCB com os valores iniciais da tarefa
    tcb->context.r0  = 0;
    tcb->context.r1  = 1;
    tcb->context.r2  = 2;
    tcb->context.r3  = 3;
    tcb->context.r4  = 4;
    tcb->context.r5  = 5;
    tcb->context.r6  = 6;
    tcb->context.r7  = 7;
    tcb->context.r8  = 8;
    tcb->context.r9  = 9;
    tcb->context.r9  = 10;
    tcb->context.r10 = 11;
    tcb->context.r11 = 12;
    tcb->context.r12 = 13;
    tcb->context.sp  = (uint32_t)&stack_mem[stack_size];    // Aponta para o final do Stack
    tcb->context.lr  = 69;                                  // Retorno em Zero
    tcb->context.cpsr = 0x10;                               // Modo Usuário
    tcb->context.pc  = (uint32_t) task_func;                           // PC inicia no "main" da task usuária

    tcb->id = id;
    tcb->stack_base = stack_mem;
    tcb->state = TASK_READY;
}

// Controle das tasks (scheduler)
/*
    Scheduler chamado pelo assembly durante o SVC
    FLOWCHART DO COISO

CPU NO SVC:
	- ALTERA MODO PRA SVC
	- CPSR (USER) É COPIADO PARA SPSR
	- ENDEREÇO DE RETORNO PÓS SVC É GUARDADO NO LR
	
	

RETORNO:
	MOVS PC, LR		// RETORNA E COLOCA O SPSR NO CPSR
(ID R7 / PARAM R0, R1, R2, R3)

*/
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
    // Se o loop terminou e ninguém foi encontrado (todos SLEEPING):
    // Forçamos a Idle Task.
    current_task = &idle_task_tcb;
}


/*
    Cria uma tarefa nova tarefa

    @param *entry_point Ponteiro para função da nova tarefa
    @param stack_size Tamanho do stack da nova tarefa, em words

    @return ID da nova função ou código de erro
*/
int32_t k_task_create(void (*entry_point)(void), size_t stack_size) {
    if (task_count >= MAX_TASKS) return -1;

    // Aloca memória para a pilha da nova task no HEAP
    // STACK_SIZE * 4 porque malloc geralmente recebe tamanho em bytes
    uint32_t *new_stack = (uint32_t *)k_malloc(stack_size * sizeof(uint32_t));

    if (new_stack == NULL){
        k_uart_print("[KERNEL] ERROR CREATING TASK: OUT OF MEMORY\n\r");
        return -2;
    } // Erro: Out of Memory na SRAM

    tcb_t *new_tcb = &task_table[task_count];

    // Inicializa passando o ponteiro do Heap
    k_task_init(new_tcb, task_count + 1, entry_point, new_stack, stack_size);

    task_count++;
    return new_tcb->id;
}

/*
    Cria uma tarefa nova tarefa

    @param *entry_point Ponteiro para função da nova tarefa
    @param stack_size Tamanho do stack da nova tarefa, em bytes

    @return ID da nova função ou código de erro
*/
int32_t k_task_create_no_interrupt(void (*entry_point)(void), size_t stack_size) {
    if (task_count >= MAX_TASKS) return -1;

    // Aloca memória para a pilha da nova task no HEAP
    // STACK_SIZE * 4 porque malloc geralmente recebe tamanho em bytes
    uint32_t *new_stack = (uint32_t *)k_malloc_no_interrupt(stack_size * sizeof(uint32_t));
    
    if (new_stack == NULL){
        k_uart_print("[KERNEL]: ERROR CREATING TASK: OUT OF MEMORY\n\r");
        return -2;
    } // Erro: Out of Memory na SRAM

    tcb_t *new_tcb = &task_table[task_count];

    // Inicializa passando o ponteiro do Heap
    k_task_init(new_tcb, task_count + 1, entry_point, new_stack, stack_size);
    
    task_count++;
    return new_tcb->id;
}

/*
    Retorna o id da tarefa atual

    @return id da tarefa
*/
uint32_t k_get_current_task_id(void) {
    if (current_task != 0) {
        return current_task->id;
    }
    return 0; // Caso não haja tarefa rodando
}


/*
    Declaração da Idle Task
*/
void idle_task_func(){
    while(1) {
        //sys_write("IDLE\n");
        asm volatile("wfi");
    }
}


/*
    Configura e inicia a Idle task 
*/
void k_idle_task_init(void) {
    uint32_t *idle_stack = (uint32_t *)k_malloc_no_interrupt(64 * sizeof(uint32_t));
    k_task_init(&idle_task_tcb, 0, idle_task_func, idle_stack, 64);
    idle_task_tcb.state = TASK_READY;
}


/*
    Handler responsável por acordar tasks dormindo
*/
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