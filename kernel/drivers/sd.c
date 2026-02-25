#include "sd.h"

static uint32_t card_rca = 0;

/* Helper: Envia comando e espera conclusão */
int sd_send_cmd(uint32_t cmd, uint32_t arg, uint32_t flags) {
    *MMCI_ARGUMENT = arg;
    // O bit 10 ativa o controlador de comando no PL181
    *MMCI_COMMAND = cmd | flags | (1 << 10);

    // Limpa o status de comando (escrita no registro de Clear em 0x38)
    *((volatile uint32_t *)(MMCI_BASE + 0x38)) = 0xFFFFFFFF;
    
    return 0;
}

int k_sd_init(void) {
    // 1. Power e Clock de Inicialização (Lento: ~400KHz)
    *MMCI_POWER = 0x03;
    for(volatile int i=0; i<10000; i++); 
    *MMCI_CLOCK = 0x1FF; 

    
    // 2. CMD0: Reset (Go Idle State)
    if (sd_send_cmd(CMD0, 0, 0)) return -1;

    // 3. CMD8: Send Interface Condition (Necessário para cartões modernos/QEMU)
    // 0x1AA verifica se o cartão suporta 2.7V-3.6V
    sd_send_cmd(CMD8, 0x1AA, 0x40); // 0x40 = espera resposta curta (R1)
    
    // 4. ACMD41: Inicialização e Verificação de Voltagem
    // Como é um ACMD, precisamos enviar CMD55 antes
    //int retry = 1000;
    //while(retry--) {
        sd_send_cmd(CMD55, 0, 0x40);
        sd_send_cmd(ACMD41, 0x40100000, 0x40); // HCS + 3.3V
        
        //if (*MMCI_RESPONSE0 & (1 << 31)) break; // Bit Busy: 1 = Pronto
    //}
    //if (retry <= 0) {k_uart_print("DESISTO\n\r"); return -1;};

    // 5. CMD2: Ask for CID (Identificação Única)
    sd_send_cmd(2, 0, 0x40 | (1 << 6)); // Resposta longa (R2)

    // 6. CMD3: Ask for RCA (Relative Card Address)
    // O cartão publica o endereço que usaremos para "falar" com ele
    sd_send_cmd(3, 0, 0x40);
    card_rca = *MMCI_RESPONSE0 & 0xFFFF0000; // RCA está nos 16 bits superiores

    // 7. CMD7: Select Card (Move para o "Transfer State")
    // ESSENCIAL: Sem isso o cartão ignora comandos de leitura de dados
    sd_send_cmd(7, card_rca, 0x40);

    // 8. CMD16: Set Block Length (512 bytes para FAT32)
    sd_send_cmd(16, 512, 0x40);

    // 9. Aumentar o Clock para velocidade de operação (25MHz ou mais)
    *MMCI_CLOCK = 0xC7; 
    //k_uart_print("SD FEITO UAI\n\r");
    return 0;
}

int k_sd_read_sector(uint32_t lba, uint8_t *buffer) {
    uint32_t *dest = (uint32_t *)buffer;

    // 1. Limpar status e configurar Data Path
    *((volatile uint32_t *)(MMCI_BASE + 0x38)) = 0xFFFFFFFF;
    *MMCI_DATA_TIMER = 0xFFFF;
    *MMCI_DATA_LEN   = 512;
    // Direção: Card para Host | Enable | Block Size 2^9 = 512
    *MMCI_DATA_CTRL  = (1 << 0) | (1 << 1) | (9 << 4);

    // 2. CMD17: Read Single Block
    // No QEMU, para SDHC (imagens > 2GB ou formatadas modernas), o arg é o LBA.
    // Se não funcionar, tente (lba * 512).
    if (sd_send_cmd(CMD17, lba*512, 0x40) != 0) {k_uart_printf("[FILESYSTEM]: SD COMMAND FAIL\n\r"); return -1;}
    // 3. Leitura da FIFO (Polling)
    int words_read = 0;
    int safety_timeout = 1000000;
    while (words_read < 128 && safety_timeout > 0) {
        uint32_t status = *MMCI_STATUS;
        // Bit 21: RXDATAAVL (Existem dados para ler na FIFO)
        if (status & (1 << 21)) {
            dest[words_read++] = *MMCI_FIFO;
        }
        // Checar erros de CRC ou Timeout de dados
        if (status & ((1 << 1) | (1 << 3))) return -1;
        safety_timeout--;
    }
        k_uart_printf("[FILESYSTEM]: SD COMMAND OK\n\r");
    return 0;
}