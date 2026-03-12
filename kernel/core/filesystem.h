#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t  jmp[3];
    char     oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  fat_count;
    uint16_t root_ent_count;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // Extensão FAT32
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} __attribute__((packed)) FAT32_BPB;

typedef struct {
    uint8_t  name[11];      // Nome 8.3
    uint8_t  attr;          // Atributos
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t cluster_high;  // Parte alta do cluster
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t cluster_low;   // Parte baixa do cluster
    uint32_t file_size;     // Tamanho em bytes
} __attribute__((packed)) DirectoryEntry;

typedef struct {
    char     name[11];
    uint32_t size;
    uint32_t first_cluster;
    uint32_t current_cluster; // Cluster onde o cursor está agora
    uint32_t fpos;            // Posição global no arquivo (0 a size)
    uint32_t sector_in_cluster; // Qual setor do cluster atual estamos
    uint8_t  buffer[512];     // Buffer de setor para otimizar pequenas leituras
    char mode[4];                // Modo atual de leitura/escrita
    bool     eof;
} FILE; // Renomeado para seguir o padrão C

// Protótipos padrão stdio
int    k_fs_init(void);
FILE* k_fopen(const char* filename, const char* mode);
size_t k_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
int    k_fclose(FILE* fp);
int    k_fgetc(FILE* fp);
char* k_fgets(char* str, int n, FILE* fp);
int    k_feof(FILE* fp);

// Funções auxiliares/administrativas
void   k_fs_ls(void);
int    k_remove(const char *filename);
#endif