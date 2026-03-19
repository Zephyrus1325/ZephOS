.section .vectors, "ax"
.global _vectors
.global _start

.include "arch/Arm/data_abort.s"
.include "arch/Arm/irq.s"
.include "arch/Arm/svc.s"
.include "arch/Arm/task_asm.s"

.global start_first_task

_vectors:
    B _start                 // 0x00: Reset (Ponto de entrada)
    B _handler_undef          // 0x04: Instrução indefinida
    B _svc_handler            // 0x08: System Call (SVC)
    B . // handler_prefetch  // 0x0C: Erro de instrução
    B _data_abort_handler     // 0x10: Erro de dados (Ex: acesso à memória inválida)
    .word 0                  // 0x14: Reservado
    B _irq_handler            // 0x18: Interrupções de hardware (Timer, etc)
    B . // handler_fiq       // 0x1C: Interrupção rápida

.section .text
_start:
    /* Configurar pilha para modo IRQ */
    MSR cpsr_c, #0xD2            // Entra no modo IRQ (Interrupções desabilitadas)
    LDR sp, =__stack_irq_top     // Define o endereço da pilha de IRQ

    LDR r0, =0x60000000
    MCR p15, 0, r0, c12, c0, 0   // Escreve no VBAR (Vector Base Address)

    /* Configurar pilha para modo Supervisor (Kernel) */
    MSR cpsr_c, #0xD3            // Entra no modo SVC
    LDR sp, =__stack_svc_top     // Define o topo da RAM para a pilha do Kernel

    // Zerar as variaveis do kernel
    LDR r0, =__bss_start
    LDR r1, =__bss_end
    MOV r2, #0
zero_loop:
    CMP r0, r1
    STRLT r2, [r0], #4
    BLT zero_loop

    BL main                      // Rodar a Main em C
    B .                          // Caso a Main retorne, trava em loop infinito

    
