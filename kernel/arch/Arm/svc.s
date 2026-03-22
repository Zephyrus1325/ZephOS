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
        - SALVA LR DO SUPERVISOR
        - SALVA R0-R12 NO STACK DO USER
        - SALVA LR, PC NO STACK DO USER
        - CARREGA O SPSR E SALVA NO STACK DO USER
        - CARREGA R0-R3 + R7 DO STACK DO SUPERVISOR
        - CHAMA O K_SVC_HANDLER
        - RETORNA
        - CARREGA LR, PC DO STACK DO USER
        - CARREGA R0-12 DO USER
        - RETORNA DA FUNÇÃO 
 */



_svc_handler:
    cpsid i
    /* 1. Ajuste do LR para retorno de IRQ */
    sub lr, lr, #4

    /* 2. Salva temporários para poder mudar de modo */
    push {r0-r3}        // Armazena os parâmetros recebidos para o SVC no stack
    mov r0, lr          // Move o PC original do usuário para r0
    mrs r1, spsr        // Move o Status original do usuário para r1

    cps #0x1F           // Entra no modo SYSTEM para acessar o stack do user

    /* 4. Monta o Frame de Contexto (Igual ao SVC) */
    push {r0}           // Salva o PC de retorno 
    push {r1}           // Salva o SPSR (CPSR da tarefa) 

    /* 5. Recupera r0-r3 originais que ficaram na pilha SVC */
    cps #0x13           // Muda para SVC
    pop {r0-r3}         // Pega os parâmetros de volta
    push {r0-r3}        // Salva para usar eles daqui a pouco
    cps #0x1F           // Volta para SYSTEM
    
    /* 6. Salva o restante dos registradores */
    push {r0-r11, r12}
    push {lr}           // Salva o LR que o usuário tinha antes (LR_usr)

    /* 7. Salva o SP atual no TCB da tarefa que foi interrompida */
    ldr r0, =current_task       //
    ldr r1, [r0]                // current_task->sp = sp;
    str sp, [r1, #4]            //

    cps #0x13                   // Volta para o Supervisor Mode
    pop {r0-r3}                 // Pega de volta os parâmetros

    /* 8. Chama o tratamento em C */
    bl k_svc_dispatcher

    cps #0x1F                   // Entra no SYSTEM Mode
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
