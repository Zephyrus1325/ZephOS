.section .text

/*
    - LOGICA DO HANDLER 2.0:
        
    - USER CHAMA SVC
    - CPU MUDA DE ESTADO
    - CPSR -> SPSR
    - LR -> LR (USER)
    - LR (NOVO) = RETORNO USER
    - ENTRA NO HANDLER
        - DESATIVA INTERRUPTS
        - SALVA R0-R3 + R7 NO STACK DO SUPERVISOR
        - SALVA R0-R12 NO STACK DO USER
        - SALVA SP, LR, PC NO STACK DO USER
        - CARREGA O SPSR E SALVA NO STACK DO USER
        - CARREGA R0-R3 + R7 DO STACK DO SUPERVISOR
        

 */



_svc_handler:
    cpsid i
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
    pop {lr}
    pop {r0-r11, r12}
    pop {r1}
    msr spsr_cxsf, r1
    cpsie i
    ldmfd sp!, {pc}^
//_svc_handler:
//    /* 1. O LR no SVC já aponta para a instrução correta. NÃO subtraia 4. */
//	cpsid i
//    /* 2. Salva o PC de retorno e o SPSR da tarefa */
//    /* Como já estamos no modo SVC, usamos a própria pilha do Kernel/Tarefa */
//    mov r12, lr         /* r12 = PC de retorno */
//    mrs r11, spsr       /* r11 = CPSR da tarefa */
//
//    /* 3. Começa a montar a struct context_t na pilha */
//    push {r11}          /* context->cpsr */
//    push {r12}          /* context->pc */
//
//    /* 4. Captura o LR do usuário (R14_usr) */
//    /* O modo SVC não enxerga o LR_usr diretamente. Usamos o modo System. */
//    cps #0x1F           /* Muda para modo System (0x1F) */
//    mov r12, lr         /* Pega o LR real do usuário */
//    cps #0x13           /* Volta para modo SVC */
//    push {r12}          /* context->lr */
//
//    /* 5. Salva o SP do usuário (R13_usr) */
//    cps #0x1F
//    mov r12, sp
//    cps #0x13
//    push {r12}          /* context->sp */
//
//    /* 6. Salva o restante dos registradores (R12 até R0) */
//    /* Note que R0-R3 originais ainda estão intactos nos registradores */
//    push {r0-r12}
//
//    /* 7. Salva o SP atual no TCB */
//    ldr r0, =current_task
//    ldr r1, [r0]
//    str sp, [r1, #4]    /* tcb->sp = sp atual */
//
//    /* 8. Chama o Dispatcher em C */
//    /* r0 = ID da syscall, r1-r3 = argumentos */
//    bl k_svc_dispatcher
//
//    /* 9. Recupera o SP (pode ter havido troca de contexto/tarefa) */
//    ldr r0, =current_task
//    ldr r1, [r0]
//    ldr sp, [r1, #4]
//
//    /* 10. Restaura o Contexto (Inverso da struct) */
//    /* Atualiza o R0 na pilha com o retorno do dispatcher */
//    str r0, [sp, #0]
//
//    pop {r0-r12}        /* Restaura R0-R12 */
//    
//    pop {r12}           /* Restaura SP_usr */
//    cps #0x1F
//    mov sp, r12
//    cps #0x13
//
//    pop {r12}           /* Restaura LR_usr */
//    cps #0x1F
//    mov lr, r12
//    cps #0x13
//
//    pop {r1}            /* r1 = PC de retorno */
//    pop {r2}            /* r2 = SPSR */
//    msr spsr_cxsf, r2
//    
//    mov lr, r1          /* Prepara o LR_svc para o retorno */
//	  cpsie i
//    ldmfd sp!, {pc}^    /* Retorno atômico */
//
