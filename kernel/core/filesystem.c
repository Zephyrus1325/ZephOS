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

int k_fs_open(const char *filename, file_t *file) {
    char fat_name[11];
    format_to_fat_name(filename, fat_name);
    
    uint8_t buffer[512*512];
    uint32_t current_lba = cluster_to_lba(bpb.root_cluster);

    //k_uart_print("ABAEDAEFASDd\n\r");
    //k_uart_print("ABAEDAEFASDd2\n\r");
    if (k_sd_read_sector(current_lba, buffer) != 0) {k_uart_print("[FILESYSTEM]: SD SECTOR READ FAIL\n\r"); return -2;}
    //k_uart_print("AAAAAAAAA\n\r");
    DirectoryEntry *entry = (DirectoryEntry *)buffer;
    for (int i = 0; i < 16; i++) {
        //k_uart_printf("Entry: 0x%x\n\r", entry[i].name[0]);
        if (entry[i].name[0] == 0x00) break;
        if (entry[i].name[0] == 0xE5) continue;

        bool match = true;
        for (int k = 0; k < 11; k++) {
            //k_uart_printf("%c | %c\n\r", entry[i].name[k], fat_name[k]);
            if (entry[i].name[k] != fat_name[k]) {
                match = false;
                //k_uart_print("AFF DEU RUIM\n\r");
                break;
            }
        }

        if (match) {
            file->size = entry[i].file_size;
            file->first_cluster = (entry[i].cluster_high << 16) | entry[i].cluster_low;
            file->buffer = (uint8_t*) k_malloc(((file->size + 511) & ~511) + 1); // Aloca o próximo multiplo de 512 bytes + 1 (O cartão SD le em blocos de 512 bytes)
            if(k_fs_read(file)){return -3;}
            return 0;
        }
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

int k_fs_save_file(const char *filename, uint8_t *data, uint32_t size) {
    uint8_t dir_buffer[512];
    
    // 1. Encontrar um cluster livre para os dados
    uint32_t free_cluster = find_free_cluster();
    if (free_cluster == 0xFFFFFFFF) {
        k_uart_print("FS: Erro - Disco Cheio\r\n");
        return -1;
    }

    // 2. Reservar o cluster na FAT (Marca como Fim de Arquivo)
    if (mark_cluster_in_fat(free_cluster, 0x0FFFFFFF) != 0) return -2;

    // 3. Gravar os dados reais no setor correspondente ao cluster
    // Importante: Estamos gravando apenas o primeiro setor do cluster (512 bytes)
    uint32_t data_lba = cluster_to_lba(free_cluster);
    if (k_sd_write_sector(data_lba, data) != 0) return -3;

    // 4. Criar a entrada no diretório raiz
    uint32_t root_lba = cluster_to_lba(bpb.root_cluster);
    if (k_sd_read_sector(root_lba, dir_buffer) != 0) return -4;

    DirectoryEntry *entries = (DirectoryEntry *)dir_buffer;
    int entry_found = -1;

    for (int i = 0; i < 16; i++) {
        // 0x00 = nunca usado, 0xE5 = deletado
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            entry_found = i;
            break;
        }
    }

    if (entry_found == -1) {
        k_uart_print("FS: Erro - Diretorio Raiz Cheio\r\n");
        return -5;
    }

    // 5. Preencher a entrada do diretório
    format_to_fat_name(filename, (char*)entries[entry_found].name);
    entries[entry_found].attr = 0x20; // Arquivo comum (Archive)
    entries[entry_found].cluster_low = (uint16_t)(free_cluster & 0xFFFF);
    entries[entry_found].cluster_high = (uint16_t)((free_cluster >> 16) & 0xFFFF);
    entries[entry_found].file_size = size;
    
    // Opcional: Aqui você preencheria crt_date e crt_time se tivesse um driver de RTC

    // 6. Gravar o setor de diretório atualizado de volta no SD
    if (k_sd_write_sector(root_lba, dir_buffer) != 0) return -6;

    k_uart_print("FS: Arquivo salvo com sucesso!\r\n");
    return 0;
}

int k_fs_create(const char *filename) {
    uint8_t dir_buffer[512];
    uint32_t root_lba = cluster_to_lba(bpb.root_cluster);
    
    // 1. Achar cluster livre na FAT (ex: cluster 500)
    uint32_t free_cluster = find_free_cluster(); 
    mark_cluster_in_fat(free_cluster, 0x0FFFFFFF);

    // 2. Ler diretório raiz para achar slot
    k_sd_read_sector(root_lba * 512, dir_buffer);
    DirectoryEntry *entries = (DirectoryEntry *)dir_buffer;
    
    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
            // Preenche o slot
            format_to_fat_name(filename, (char*)entries[i].name);
            entries[i].attr = 0x20;
            entries[i].cluster_low = free_cluster & 0xFFFF;
            entries[i].cluster_high = (free_cluster >> 16) & 0xFFFF;
            entries[i].file_size = 0; // Começa vazio
            
            // Grava o setor de diretório de volta no SD
            k_sd_write_sector(root_lba, dir_buffer);
            return 0;
        }
    }
    return -1;
}