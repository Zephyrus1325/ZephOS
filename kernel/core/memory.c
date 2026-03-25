#include "drivers/interrupts.h"
#include "core/memory.h"

extern uint8_t __heap_start;
extern uint8_t __heap_end;

static block_header_t *free_list = NULL;

/* Função auxiliar para inicializar o Heap na SRAM */
void k_heap_init() {
    free_list = (block_header_t *)&__heap_start;
    free_list->size = (uintptr_t)&__heap_end - (uintptr_t)&__heap_start - HEADER_SIZE;
    free_list->is_free = 1;
    free_list->next = NULL;
}


/*
    Aloca um bloco de memória dentro do Heap
    
    @param  size Número de bytes a serem alocados
    @return Ponteiro para o início do bloco alocado, retorna NULL em caso de erro
*/
void* k_malloc(size_t size) {
    // 1. Alinhamento de 8 bytes
    size = (size + 7) & ~7;

    //k_disable_interrupts(); // Desabilitar interrupções para evitar race condition

    block_header_t *curr = free_list;


    // Procurar o primeiro bloco livre que sirva (First Fit)
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            
            // Se o bloco for muito maior que o necessário, dividi-lo (Split)
            if (curr->size > (size + HEADER_SIZE + 8)) {
                block_header_t *next_block = (block_header_t *)((uint8_t *)curr + HEADER_SIZE + size);
                next_block->size = curr->size - size - HEADER_SIZE;
                next_block->is_free = 1;
                next_block->next = curr->next;

                curr->size = size;
                curr->next = next_block;
            }

            curr->is_free = 0;

            //k_enable_interrupts();   // Reativa Interrupções
            return (void *)((uint8_t *)curr + HEADER_SIZE);
        }
        curr = curr->next;
    }

    //k_enable_interrupts();  // Reativa Interrupções
    return NULL; // Sem memória
}

/*
    Aloca um bloco de memória dentro do Heap
    
    @param  size Número de bytes a serem alocados
    @return Ponteiro para o início do bloco alocado, retorna NULL em caso de erro
*/
void* k_malloc_no_interrupt(size_t size) {
    // 1. Alinhamento de 8 bytes
    size = (size + 7) & ~7;

    block_header_t *curr = free_list;


    // Procurar o primeiro bloco livre que sirva (First Fit)
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            
            // Se o bloco for muito maior que o necessário, dividi-lo (Split)
            if (curr->size > (size + HEADER_SIZE + 8)) {
                block_header_t *next_block = (block_header_t *)((uint8_t *)curr + HEADER_SIZE + size);
                next_block->size = curr->size - size - HEADER_SIZE;
                next_block->is_free = 1;
                next_block->next = curr->next;

                curr->size = size;
                curr->next = next_block;
            }

            curr->is_free = 0;

            return (void *)((uint8_t *)curr + HEADER_SIZE);
        }
        curr = curr->next;
    }

    return NULL; // Sem memória
}


/* 
    Libera um bloco de memória

    @param *ptr ponteiro para o início do bloco a ser liberado
*/
void k_free(void *ptr) {
    if (!ptr) return;

    k_disable_interrupts();   // Desabilitar interrupções para evitar race condition

    // 1. Encontrar o header do bloco
    block_header_t *header = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    header->is_free = 1;

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

    k_enable_interrupts();
}


/* 
    Retorna o total de bytes livres no Heap 

    @return Número de bytes livres
*/
size_t k_get_free_heap() {
    size_t total_free = 0;
    
    k_disable_interrupts(); // Desabilitar interrupções para evitar race condition

    block_header_t *curr = free_list;
    while (curr) {
        if (curr->is_free) {
            total_free += curr->size;
        }
        curr = curr->next;
    }

    k_enable_interrupts();
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


/* 
    Retorna o maior bloco contínuo disponível (importante para saber se um malloc grande vai falhar) 

    @return Tamanho do maior bloco livre em bytes
*/
size_t k_get_max_free_block() {
    size_t max_block = 0;
    
    k_disable_interrupts();

    block_header_t *curr = free_list;
    while (curr) {
        if (curr->is_free && curr->size > max_block) {
            max_block = curr->size;
        }
        curr = curr->next;
    }

    k_enable_interrupts();
    return max_block;
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint32_t* d32 = (uint32_t*)dest;
    const uint32_t* s32 = (const uint32_t*)src;

    // Se a origem e o destino estiverem alinhados em 4 bytes
    if (((uintptr_t)dest % 4 == 0) && ((uintptr_t)src % 4 == 0)) {
        // Copia blocos de 4 bytes
        while (n >= 4) {
            *d32++ = *s32++;
            n -= 4;
        }
    }

    // Copia o que sobrar (ou se não estiver alinhado) byte a byte
    uint8_t* d8 = (uint8_t*)d32;
    uint8_t* s8 = (uint8_t*)s32;

    while (n > 0) {
        *d8++ = *s8++;
        n--;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint32_t *p1_32 = (const uint32_t *)s1;
    const uint32_t *p2_32 = (const uint32_t *)s2;

    // Se ambos os ponteiros estiverem alinhados em 4 bytes (32-bit boundary)
    if (((uintptr_t)s1 % 4 == 0) && ((uintptr_t)s2 % 4 == 0)) {
        while (n >= 4) {
            if (*p1_32 != *p2_32) {
                // Se encontrar diferença na word, sai para comparar byte a byte
                break;
            }
            p1_32++;
            p2_32++;
            n -= 4;
        }
    }

    // Compara o restante (ou se não estiver alinhado) byte a byte
    const unsigned char *p1_8 = (const unsigned char *)p1_32;
    const unsigned char *p2_8 = (const unsigned char *)p2_32;

    while (n--) {
        if (*p1_8 != *p2_8) {
            return (int)(*p1_8 - *p2_8);
        }
        p1_8++;
        p2_8++;
    }

    return 0;
}