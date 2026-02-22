#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

typedef struct block_header {
    size_t size;                /* Tamanho do bloco de dados (excluindo este header) */
    int is_free;                /* 1 se o bloco estiver livre, 0 se ocupado */
    struct block_header *next;  /* Ponteiro para o próximo bloco na SRAM */
} block_header_t;

#define HEADER_SIZE sizeof(block_header_t)

void k_heap_init();
void* k_malloc(size_t size);
void k_free(void *ptr);
size_t k_get_free_heap();
size_t k_get_max_free_block();
size_t k_get_free_heap_no_interrupt();

#endif