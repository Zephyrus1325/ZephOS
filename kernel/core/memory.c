#include "interrupts.h"
#include "memory.h"

extern uint8_t __heap_start;
extern uint8_t __heap_limit;

static block_header_t *free_list = NULL;

/* Função auxiliar para inicializar o Heap na SRAM */
void k_heap_init() {
    free_list = (block_header_t *)&__heap_start;
    free_list->size = (uintptr_t)&__heap_limit - (uintptr_t)&__heap_start - HEADER_SIZE;
    free_list->is_free = 1;
    free_list->next = NULL;
}

void* k_malloc(size_t size) {
    // 1. Alinhamento de 8 bytes
    size = (size + 7) & ~7;

    // 2. Bloquear interrupções (Seção Crítica)
    // Aqui deves usar uma função que desative o IRQ no CPSR
    //disable_interrupts(); 

    block_header_t *curr = free_list;

    // 3. Procurar o primeiro bloco livre que sirva (First Fit)
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            
            // 4. Se o bloco for muito maior que o necessário, dividi-lo (Split)
            if (curr->size > (size + HEADER_SIZE + 8)) {
                block_header_t *next_block = (block_header_t *)((uint8_t *)curr + HEADER_SIZE + size);
                next_block->size = curr->size - size - HEADER_SIZE;
                next_block->is_free = 1;
                next_block->next = curr->next;

                curr->size = size;
                curr->next = next_block;
            }

            curr->is_free = 0;
            //enable_interrupts();
            return (void *)((uint8_t *)curr + HEADER_SIZE);
        }
        curr = curr->next;
    }

    //enable_interrupts();
    return NULL; // Sem memória
}

void k_free(void *ptr) {
    if (!ptr) return;

    //disable_interrupts();

    // 1. Encontrar o header do bloco
    block_header_t *header = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    header->is_free = 1;

    // 2. Coalescência (Opcional mas recomendado): 
    // Juntar blocos livres adjacentes para evitar fragmentação
    block_header_t *curr = free_list;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += curr->next->size + HEADER_SIZE;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    //enable_interrupts();
}

/* Retorna o total de bytes livres no Heap */
size_t k_get_free_heap() {
    size_t total_free = 0;
    
    // É vital desabilitar interrupções se houver chance de outra task
    // chamar k_malloc ou k_free enquanto percorremos a lista.
    disable_interrupts(); 

    block_header_t *curr = free_list;
    while (curr) {
        if (curr->is_free) {
            total_free += curr->size;
        }
        curr = curr->next;
    }

    enable_interrupts();
    return total_free;
}

size_t k_get_free_heap_no_interrupt() {
    size_t total_free = 0;

    block_header_t *curr = free_list;
    while (curr) {
        if (curr->is_free) {
            total_free += curr->size;
        }
        curr = curr->next;
    }

    return total_free;
}

/* Retorna o maior bloco contínuo disponível (importante para saber se um malloc grande vai falhar) */
size_t k_get_max_free_block() {
    size_t max_block = 0;
    
    disable_interrupts();
    block_header_t *curr = free_list;
    while (curr) {
        if (curr->is_free && curr->size > max_block) {
            max_block = curr->size;
        }
        curr = curr->next;
    }
    enable_interrupts();
    return max_block;
}