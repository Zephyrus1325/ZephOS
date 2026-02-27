#ifndef SD_H
#define SD_H

#include <stdint.h>
#include <stdbool.h>

#define MMCI_BASE       0x10005000

/* Registradores PL181 */
#define MMCI_POWER      ((volatile uint32_t *)(MMCI_BASE + 0x000))
#define MMCI_CLOCK      ((volatile uint32_t *)(MMCI_BASE + 0x004))
#define MMCI_ARGUMENT   ((volatile uint32_t *)(MMCI_BASE + 0x008))
#define MMCI_COMMAND    ((volatile uint32_t *)(MMCI_BASE + 0x00C))
#define MMCI_RESP_CMD   ((volatile uint32_t *)(MMCI_BASE + 0x010))
#define MMCI_RESPONSE0  ((volatile uint32_t *)(MMCI_BASE + 0x014))
#define MMCI_DATA_TIMER ((volatile uint32_t *)(MMCI_BASE + 0x024))
#define MMCI_DATA_LEN   ((volatile uint32_t *)(MMCI_BASE + 0x028))
#define MMCI_DATA_CTRL  ((volatile uint32_t *)(MMCI_BASE + 0x02C))
#define MMCI_STATUS     ((volatile uint32_t *)(MMCI_BASE + 0x034))
#define MMCI_FIFO       ((volatile uint32_t *)(MMCI_BASE + 0x080))

/* Comandos SD Comuns */
#define CMD0   0  /* GO_IDLE_STATE */
#define CMD8   8  /* SEND_IF_COND */
#define CMD16  16 /* SET_BLOCKLEN */
#define CMD17  17 /* READ_SINGLE_BLOCK */
#define CMD24  24 /* WRITE_SINGLE_BLOCK*/
#define ACMD41 41 /* SD_SEND_OP_COND */
#define CMD55  55 /* APP_CMD (Prefixo para ACMD) */

/* Status Bits */
#define STAT_CMD_DONE   (1 << 2)
#define STAT_DATA_END   (1 << 8)
#define STAT_RX_FIFO_HF (1 << 10) // FIFO Half Full

/* Protótipos */
int k_sd_init(void);
int k_sd_read_sector(uint32_t lba, uint8_t *buffer);
int k_sd_write_sector(uint32_t lba, uint8_t *buffer);

#endif