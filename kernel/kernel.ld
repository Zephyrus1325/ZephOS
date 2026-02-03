/* Define o ponto de entrada como o símbolo _start definido no startup.s */
ENTRY(_start)

MEMORY
{
    /* Memória Principal (SDRAM) - 1GB */
    SDRAM (rwx) : ORIGIN = 0x00000000, LENGTH = 1024M
    
    /* SRAM do FPGA (Mapeada via HPS Bridge) - 64MB */
    SRAM  (rwx) : ORIGIN = 0xC0000000, LENGTH = 64M
}


SECTIONS
{
    /* 1. Código e Dados Estáticos na SDRAM */
    . = 0x00000000;

    .text : {
        KEEP(*(.vectors))
        *(.text)
        *(.rodata)
    } > SDRAM

    .data : {
        *(.data)
    } > SDRAM

    .bss : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > SDRAM

    /* 2. Reserva de Espaço para Sistema de Arquivos na SDRAM */
    /* Vamos colocar após o BSS, reservando 512MB */
    . = ALIGN(4096); /* Alinha em página para futura MMU */
    __fs_start = .;
    . = . + 512M;
    __fs_end = .;

    /* 3. Definição da Pilha (Stack) no final da SDRAM */
    /* Colocamos no final da SDRAM para ficar longe do código */
    _stack_top = ORIGIN(SDRAM) + LENGTH(SDRAM);

    /* 4. HEAP na SRAM (0xC0000000) */
    .sram_data (NOLOAD) : {
        . = ALIGN(8);
        __heap_start = .;
        /* O limite é o fim da região SRAM */
        __heap_limit = ORIGIN(SRAM) + LENGTH(SRAM);
    } > SRAM
}