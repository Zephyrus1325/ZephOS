#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stdbool.h>

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
    char name[32];
    uint32_t size;
    uint32_t first_cluster;
    uint8_t* buffer;
    bool is_directory;
} file_t;

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

/* Retornos: 0 para Sucesso, -1 para Erro */

int k_fs_init(void);
void k_fs_ls(void);
int k_fs_open(const char *filename, file_t *file);
int k_fs_read(file_t *file);

#endif