#include "core/filesystem.h"
#include "drivers/sd.h"   // Certifique-se de que o path está correto
#include "drivers/uart.h"
#include "core/memory.h"

static FAT32_BPB bpb;
static uint32_t partition_begin_lba = 0;

static uint32_t cluster_to_lba(uint32_t cluster) {
    uint32_t fat_region_size = bpb.fat_count * bpb.sectors_per_fat;
    uint32_t data_region_lba = partition_begin_lba + bpb.reserved_sectors + fat_region_size;
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

static uint32_t get_next_cluster(uint32_t current_cluster) {
    uint8_t fat_buffer[512];
    uint32_t fat_offset = current_cluster * 4;
    uint32_t fat_sector = partition_begin_lba + bpb.reserved_sectors + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;

    if (k_sd_read_sector(fat_sector, fat_buffer) != 0) return 0x0FFFFFFF;
    uint32_t next = *(uint32_t *)&fat_buffer[ent_offset];
    return next & 0x0FFFFFFF;
}

int k_fs_init(void) {
    uint8_t sector_buffer[512];

    if (k_sd_init() != 0) {
        k_uart_print("[FILESYSTEM]: SD DRIVER ERROR\r\n");
        return -1;
    }

    if (k_sd_read_sector(0, sector_buffer) != 0) return -1;

    // Checar MBR
    if (sector_buffer[450] == 0x0C || sector_buffer[450] == 0x0B) {
        partition_begin_lba = *(uint32_t*)&sector_buffer[454];
        if (k_sd_read_sector(partition_begin_lba, sector_buffer) != 0) return -1;
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

int k_fs_open(const char *filename, file_t *file) {
    char fat_name[11];
    format_to_fat_name(filename, fat_name);
    
    uint8_t buffer[512];
    uint32_t current_lba = cluster_to_lba(bpb.root_cluster);

    k_uart_printf("ABAEDAEFASDd\n\r");

    if (k_sd_read_sector(current_lba, buffer) != 0) {k_uart_printf("[FILESYSTEM]: SD SECTOR READ FAIL\n\r"); return -2;}
    k_uart_printf("AAAAAAAAA\n\r");
    DirectoryEntry *entry = (DirectoryEntry *)buffer;
    for (int i = 0; i < 16; i++) {
        k_uart_printf("Entry: 0x%x\n\r", entry[i].name[0]);
        if (entry[i].name[0] == 0x00) break;
        if (entry[i].name[0] == 0xE5) continue;

        bool match = true;
        for (int k = 0; k < 11; k++) {
            if (entry[i].name[k] != fat_name[k]) {
                match = false;
                break;
            }
        }

        if (match) {
            file->size = entry[i].file_size;
            file->first_cluster = (entry[i].cluster_high << 16) | entry[i].cluster_low;
            k_uart_print("aqui vai\n\r");
            file->buffer = (uint8_t*) k_malloc(file->size);
            k_uart_print("aqui ?\n\r");
            if(k_fs_read(file)){return -3;}
            return 0;
        }
        if(i >= 10){k_uart_printf("NOME NORMAL: %s\n\rNOME FAT32: %s\n\r", filename, fat_name);}
    }
    return -4;
}

int k_fs_close(file_t* file){
    k_free(file->buffer);
}

int k_fs_read(file_t *file) {
    
    uint32_t current_cluster = file->first_cluster;
    uint32_t buffer_offset = 0;

    while (current_cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(current_cluster);
        for (int s = 0; s < bpb.sectors_per_cluster; s++) {
            if (k_sd_read_sector(lba + s, file->buffer + buffer_offset) != 0) return -1;
            buffer_offset += 512;
        }
        
        current_cluster = get_next_cluster(current_cluster);
    }
    return 0;
}

void k_fs_ls() {
    uint8_t buffer[512];
    if (k_sd_read_sector(cluster_to_lba(bpb.root_cluster), buffer) != 0) return;
    DirectoryEntry *entry = (DirectoryEntry *)buffer;

    k_uart_print("--- Files ---\r\n");
    for (int i = 0; i < 16; i++) {
        if (entry[i].name[0] == 0x00) break;
        if (entry[i].name[0] == 0xE5 || (entry[i].attr & 0x0F) == 0x0F) continue;

        for (int k = 0; k < 11; k++) {
            k_uart_putc(entry[i].name[k]);
            if (k == 7) k_uart_putc(' '); 
        }
        k_uart_print("\r\n");
    }
}