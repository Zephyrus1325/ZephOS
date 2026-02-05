/* Define o ponto de entrada do binário */
ENTRY(_start)

/* Definição das regiões de memória física */
MEMORY
{
    /* RAM na Vexpress-A9 começa em 0x60000000. 
       Definimos 512MB, mas você pode ajustar conforme o comando -m do QEMU */
    RAM (rwx) : ORIGIN = 0x60000000, LENGTH = 512M
}

/* Definição de Símbolos Úteis para o Stack (Pilha) */
/* O ARM exige pilhas separadas para diferentes modos de exceção */
_stack_size_svc = 0x4000;    /* 16KB para o modo Supervisor (Main) */
_stack_size_irq = 0x1000;    /* 4KB para o modo IRQ */
_stack_size_abt = 0x400;     /* 1KB para Aborts */

SECTIONS
{
    /* 1. SEÇÃO DE CÓDIGO (.text) */
    /* Colocamos no início da RAM para facilitar o boot */
    .text : 
    {
        FILL(0x00)
        __text_start = .;
        
        /* A tabela de vetores DEVE estar no início para o processador achar */
        KEEP(*(.vectors))   
        
        *(.text .text.*)     /* Todo o código C e Assembly */
        *(.rodata .rodata.*) /* Strings e constantes */
        
        . = ALIGN(4);
        __text_end = .;
    } > RAM

    /* 2. SEÇÃO DE DADOS INICIALIZADOS (.data) */
    /* Variáveis globais como: int x = 10; */
    .data : 
    {
        __data_start = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM

    /* 3. SEÇÃO DE DADOS NÃO INICIALIZADOS (.bss) */
    /* Variáveis globais como: int y; (Devem ser zeradas no startup.s) */
    .bss (NOLOAD) : 
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)            /* Itens comuns do compilador */
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    /* 4. SEÇÃO DE PILHAS (STACKS) */
    /* Alinhamento de 8 bytes é obrigatório pela ABI do ARM */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        
        /* Pilha do modo IRQ */
        __stack_irq_start = .;
        . = . + _stack_size_irq;
        __stack_irq_top = .;

        /* Pilha do modo Supervisor (Principal) */
        __stack_svc_start = .;
        . = . + _stack_size_svc;
        __stack_svc_top = .;
        
        . = ALIGN(8);
    } > RAM

    /* 5. HEAP (Memória Dinâmica) */
    /* Define o que sobrou da RAM para uso com malloc() */
    .heap (NOLOAD) :
    {
        . = ALIGN(4096);     /* Alinhado por página para MMU futura */
        __heap_start = .;
        __heap_end = ORIGIN(RAM) + LENGTH(RAM);
    } > RAM

    /* Símbolos para debug ou uso no Kernel */
    _end = .;
}