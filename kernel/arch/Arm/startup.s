.section .vectors, "ax"
.global _vectors
.global _start

_vectors:
    B _start                 // 0x00: Reset (Ponto de entrada)
    B . // handler_undef     // 0x04: Instrução indefinida
    B svc_handler            // 0x08: System Call (SVC)
    B . // handler_prefetch  // 0x0C: Erro de instrução
    B data_abort_handler     // 0x10: Erro de dados (Ex: acesso à memória inválida)
    .word 0                  // 0x14: Reservado
    B irq_handler            // 0x18: Interrupções de hardware (Timer, etc)
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

// --- HABILITAR FPU E NEON ---
    // 1. Permitir acesso aos coprocessadores CP10 e CP110
    //mrc p15, 0, r0, c1, c0, 2
    //orr r0, r0, #0x00f00000    // Habilita acesso Full
    //mcr p15, 0, r0, c1, c0, 2
    //isb                        // Barreira de instrução

    // 2. Ligar a unidade NEON/VFP (bit 30 do FPEXC)
    //mov r0, #0x40000000
    //vmsr fpexc, r0             // Se falhar aqui, use: mcr p10, 7, r0, cr8, cr0, 0

    BL main                      // Rodar a Main em C
    B .                          // Caso a Main retorne, trava em loop infinito

svc_handler:
    /* No SVC, o LR não precisa de ajuste */
    push {lr}           /* Salva PC de retorno */
    mrs r12, spsr
    push {r12}          /* Salva SPSR (CPSR da tarefa) */

    push {r0-r11, r12}  /* Salva registradores */
    push {lr}           /* Salva LR original da tarefa */

    ldr r4, =current_task
    ldr r5, [r4]
    str sp, [r5, #4]

    bl k_svc_dispatcher

    ldr r4, =current_task
    ldr r5, [r4]
    ldr sp, [r5, #4]

    str r0, [sp, #4]

    pop {lr}
    pop {r0-r11, r12}
    pop {r1}
    msr spsr_cxsf, r1
    ldmfd sp!, {pc}^

irq_handler:
    /* 1. Ajuste do LR para retorno de IRQ */
    sub lr, lr, #4

    /* 2. Salva temporários para poder mudar de modo */
    push {r0-r3}
    mov r0, lr          /* r0 agora guarda o PC de retorno da tarefa */
    mrs r1, spsr        /* r1 agora guarda o CPSR da tarefa */

    /* 3. Muda para modo SVC para salvar na pilha da tarefa */
    cps #0x13

    /* 4. Monta o Frame de Contexto (Igual ao SVC) */
    push {r0}           /* Salva o PC de retorno */
    push {r1}           /* Salva o SPSR (CPSR da tarefa) */
    
    /* 5. Recupera r0-r3 originais que ficaram na pilha IRQ */
    cps #0x12
    pop {r0-r3}
    cps #0x13
    
    /* 6. Salva o restante dos registradores */
    push {r0-r11, r12}
    push {lr}           /* Salva o LR original da tarefa (R14_usr) */

    /* 7. Salva o SP atual no TCB da tarefa que foi interrompida */
    ldr r0, =current_task
    ldr r1, [r0]
    str sp, [r1, #4]

    /* 8. Chama o tratamento em C (Tick + Scheduler) */
    bl k_irq_handler

    /* 9. Carrega o SP da nova tarefa (pode ser a Idle ou outra) */
    ldr r0, =current_task
    ldr r1, [r0]
    ldr sp, [r1, #4]

    /* 10. Restaura o Contexto (Simétrico) */
    pop {lr}
    pop {r0-r11, r12}
    pop {r1}            /* r1 = SPSR */
    msr spsr_cxsf, r1
    ldmfd sp!, {pc}^           /* Restaura PC e CPSR simultaneamente */

data_abort_handler:
    // 1. Recuperar o endereço da instrução que falhou (LR - 8)
    SUB LR, LR, #8
    
    // 2. Ler o endereço que causou a falha (DFAR)
    MRC p15, 0, R0, c6, c0, 0  // R0 = Endereço da falha
    
    // 3. Ler o motivo da falha (DFSR)
    MRC p15, 0, R1, c5, c0, 0  // R1 = Status/Motivo
    
    // 4. Passar para uma função C para imprimir bonitinho
    // O LR (instrução) vai como 3º argumento (R2)
    MOV R2, LR
    B k_panic_data_abort


.global start_first_task
start_first_task:
    /* 9. Carrega o SP da nova tarefa (pode ser a Idle ou outra) */
    ldr r0, =current_task
    ldr r1, [r0]
    ldr sp, [r1, #4]

    /* 10. Restaura o Contexto (Simétrico) */
    pop {lr}
    pop {r0-r11, r12}
    pop {r1}            /* r1 = SPSR */
    msr spsr_cxsf, r1

    cpsie i

    ldmfd sp!, {pc}^           /* Restaura PC e CPSR simultaneamente */
