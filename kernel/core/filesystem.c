#include "core/filesystem.h"
#include "drivers/sd.h"
#include "core/memory.h"
#include <string.h>
#include "drivers/uart.h"

static FAT32_BPB bpb;
static uint32_t partition_begin_lba = 0;

// Helper: Converte cluster para LBA
static uint32_t cluster_to_lba(uint32_t cluster) {
    if (cluster < 2) return partition_begin_lba + bpb.reserved_sectors;
    uint32_t data_region_lba = partition_begin_lba + bpb.reserved_sectors + (bpb.fat_count * bpb.sectors_per_fat);
    return data_region_lba + ((cluster - 2) * bpb.sectors_per_cluster);
}

static void format_to_fat_name(const char *src, char *dest) {
    int i = 0, j = 0;
    for (i = 0; i < 11; i++) dest[i] = ' ';
    i = 0;
    while (src[i] != '.' && src[i] != '\0' && j < 8) {
        dest[j++] = (src[i] >= 'a' && src[i] <= 'z') ? src[i] - 32 : src[i];
        i++;
    }
    if (src[i] == '.') {
        i++;
        j = 8;
        while (src[i] != '\0' && j < 11) {
            dest[j++] = (src[i] >= 'a' && src[i] <= 'z') ? src[i] - 32 : src[i];
            i++;
        }
    }
}


// Helper: Próximo cluster na FAT
static uint32_t get_next_cluster(uint32_t current) {
    uint8_t fat_sector_data[512];
    uint32_t fat_offset = current * 4;
    uint32_t lba = partition_begin_lba + bpb.reserved_sectors + (fat_offset / 512);
    if (k_sd_read_sector(lba, fat_sector_data) != 0) return 0x0FFFFFFF;
    return (*(uint32_t*)&fat_sector_data[fat_offset % 512]) & 0x0FFFFFFF;
}

int mark_cluster_in_fat(uint32_t cluster, uint32_t value) {
    uint8_t fat_buffer[512];
    
    // 1. Calcular em qual setor da FAT o índice desse cluster reside
    // Cada setor tem 512 bytes, cada entrada tem 4 bytes = 128 entradas por setor.
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector_relative = fat_offset / 512;
    uint32_t entry_offset = fat_offset % 512;

    // 2. Atualizar todas as cópias da FAT (geralmente bpb.fat_count = 2)
    for (int i = 0; i < bpb.fat_count; i++) {
        uint32_t fat_lba = partition_begin_lba + bpb.reserved_sectors + 
                           (i * bpb.sectors_per_fat) + fat_sector_relative;

        // Lê o setor atual da FAT
        if (k_sd_read_sector(fat_lba, fat_buffer) != 0) return -1;

        // Altera apenas os 4 bytes correspondentes ao cluster
        uint32_t *entries = (uint32_t *)fat_buffer;
        entries[entry_offset / 4] = value & 0x0FFFFFFF; // FAT32 usa 28 bits

        // Escreve de volta o setor modificado
        if (k_sd_write_sector(fat_lba, fat_buffer) != 0) return -1;
    }

    return 0;
}

uint32_t find_free_cluster() {
    uint8_t fat_buffer[512];
    uint32_t total_fat_sectors = bpb.sectors_per_fat;

    for (uint32_t s = 0; s < total_fat_sectors; s++) {
        uint32_t lba = partition_begin_lba + bpb.reserved_sectors + s;
        if (k_sd_read_sector(lba, fat_buffer) != 0) return 0xFFFFFFFF;

        uint32_t *entries = (uint32_t *)fat_buffer;
        for (int i = 0; i < 128; i++) {
            // Pula os clusters reservados (0 e 1) no primeiro setor da FAT
            if (s == 0 && i < 2) continue;

            if ((entries[i] & 0x0FFFFFFF) == 0) {
                return (s * 128) + i;
            }
        }
    }
    return 0xFFFFFFFF; // Disco cheio
}

int k_fs_init(void) {
    uint8_t sector_buffer[512];

    if (k_sd_init() != 0) {
        k_uart_print("[FILESYSTEM]: SD DRIVER ERROR\r\n");
        return -1;
    }

    if (k_sd_read_sector(0, sector_buffer) != 0){k_uart_print("[FILESYSTEM]: SD SECTOR READ FAILED [!!!]\n\r"); return -1;}
    // Checar MBR
    if (sector_buffer[450] == 0x0C || sector_buffer[450] == 0x0B) {
        partition_begin_lba = *(uint32_t*)&sector_buffer[454];
        if (k_sd_read_sector(partition_begin_lba, sector_buffer) != 0){k_uart_print("[FILESYSTEM]: SD SECTOR READ FAILED [!!!]\n\r"); return -1;}
    } else {
        partition_begin_lba = 0;
    }

    for (int i = 0; i < sizeof(FAT32_BPB); i++) {
        ((uint8_t*)&bpb)[i] = sector_buffer[i];
    }

    if (bpb.boot_signature != 0x29) {
        k_uart_print("[FILESYSTEM]: INVALID FAT32\r\n");
        return -1;
    }
    return 0;
}


FILE* k_fopen(const char* filename, const char* mode) {
    char fat_name[11];
    format_to_fat_name(filename, fat_name);

    uint8_t buffer[512];
    uint32_t root_lba = cluster_to_lba(bpb.root_cluster);
    
    // Por enquanto, busca apenas no root directory
    if (k_sd_read_sector(root_lba, buffer) != 0) return NULL;

    DirectoryEntry *entries = (DirectoryEntry *)buffer;
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00) break;
        if (memcmp(entries[i].name, fat_name, 11) == 0) {
            FILE* fp = (FILE*)k_malloc(sizeof(FILE));
            fp->size = entries[i].file_size;
            fp->first_cluster = (entries[i].cluster_high << 16) | entries[i].cluster_low;
            fp->current_cluster = fp->first_cluster;
            fp->fpos = 0;
            fp->eof = false;
            return fp;
        }
    }
    return NULL;
}

size_t k_fread(void* ptr, size_t size, size_t nmemb, FILE* fp) {
    if (!fp || fp->eof) return 0;

    size_t total_to_read = size * nmemb;
    if (fp->fpos + total_to_read > fp->size) {
        total_to_read = fp->size - fp->fpos;
    }

    uint8_t* out = (uint8_t*)ptr;
    size_t bytes_read = 0;

    while (bytes_read < total_to_read) {
        uint32_t lba = cluster_to_lba(fp->current_cluster);
        uint32_t offset_in_cluster = fp->fpos % (bpb.sectors_per_cluster * 512);
        uint32_t sector_offset = (offset_in_cluster / 512);
        uint32_t byte_offset = (offset_in_cluster % 512);

        // Lê o setor atual para o buffer interno do arquivo
        if (k_sd_read_sector(lba + sector_offset, fp->buffer) != 0) break;

        // Calcula quanto podemos ler deste setor carregado
        size_t can_read = 512 - byte_offset;
        if (can_read > (total_to_read - bytes_read)) {
            can_read = total_to_read - bytes_read;
        }

        memcpy(out + bytes_read, fp->buffer + byte_offset, can_read);
        
        bytes_read += can_read;
        fp->fpos += can_read;

        // Se chegamos ao fim do cluster, pula para o próximo
        if (fp->fpos % (bpb.sectors_per_cluster * 512) == 0) {
            fp->current_cluster = get_next_cluster(fp->current_cluster);
            if (fp->current_cluster >= 0x0FFFFFF8) {
                fp->eof = true;
                break;
            }
        }
    }

    return bytes_read / size;
}

int k_fgetc(FILE* fp) {
    unsigned char c;
    if (k_fread(&c, 1, 1, fp) == 1) return (int)c;
    return -1; // EOF
}

char* k_fgets(char* str, int n, FILE* fp) {
    int i = 0;
    while (i < n - 1) {
        int c = k_fgetc(fp);
        if (c == -1) {
            if (i == 0) return NULL;
            break;
        }
        str[i++] = (char)c;
        if (c == '\n') break;
    }
    str[i] = '\0';
    return str;
}

int k_fclose(FILE* fp) {
    if (fp) {
        k_free(fp);
        return 0;
    }
    return -1;
}

int k_feof(FILE* fp) {
    return fp->eof || (fp->fpos >= fp->size);
}

void free_cluster_chain(uint32_t start_cluster) {
    uint32_t current = start_cluster;
    uint32_t next;
    uint8_t fat_buffer[512];

    while (current >= 2 && current < 0x0FFFFFF8) {
        // 1. Descobrir qual é o próximo cluster antes de apagar o atual
        uint32_t fat_offset = current * 4;
        uint32_t fat_sector = partition_begin_lba + bpb.reserved_sectors + (fat_offset / 512);
        uint32_t entry_offset = fat_offset % 512;

        if (k_sd_read_sector(fat_sector, fat_buffer) != 0) break;
        
        next = (*(uint32_t *)&fat_buffer[entry_offset]) & 0x0FFFFFFF;

        // 2. Marcar o cluster atual como livre (0)
        mark_cluster_in_fat(current, 0x00000000);

        // 3. Mover para o próximo
        current = next;
    }
}


int k_remove(const char *filename) {
    char fat_name[11];
    format_to_fat_name(filename, fat_name);
    
    uint8_t dir_buffer[512];
    uint32_t root_lba = cluster_to_lba(bpb.root_cluster);

    if (k_sd_read_sector(root_lba, dir_buffer) != 0) return -1;

    DirectoryEntry *entries = (DirectoryEntry *)dir_buffer;
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00) break; // Fim da lista
        if (entries[i].name[0] == 0xE5) continue; // Já deletado

        // Comparar nome
        bool match = true;
        for (int k = 0; k < 11; k++) {
            if (entries[i].name[k] != fat_name[k]) {
                match = false;
                break;
            }
        }

        if (match) {
            // 1. Obter o primeiro cluster do arquivo
            uint32_t first_cluster = (entries[i].cluster_high << 16) | entries[i].cluster_low;

            // 2. Liberar todos os clusters na FAT
            free_cluster_chain(first_cluster);

            // 3. Marcar a entrada do diretório como deletada (0xE5)
            entries[i].name[0] = 0xE5;

            // 4. Salvar o setor de diretório de volta no SD
            if (k_sd_write_sector(root_lba, dir_buffer) != 0) return -2;

            return 0;
        }
    }

    // Arquivo para excluir nao encontrado
    return -3;
}