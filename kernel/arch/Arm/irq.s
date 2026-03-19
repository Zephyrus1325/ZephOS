.section .text

_irq_handler:
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
