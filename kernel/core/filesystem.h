#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char name[32];
    uint32_t size;
    uint32_t first_cluster;
    bool is_directory;
} file_t;

/**
 * @brief Inicializa o sistema de arquivos (lê o Superblock/BPB)
 */
bool fs_init(void);

/**
 * @brief Lista arquivos no diretório atual
 */
void fs_ls(void);

/**
 * @brief Abre um arquivo pelo nome
 * @return true se encontrado, preenche a struct file
 */
bool fs_open(const char *filename, file_t *file);

/**
 * @brief Lê conteúdo de um arquivo para um buffer
 */
bool fs_read(file_t *file, uint8_t *buffer, uint32_t length);

#endif