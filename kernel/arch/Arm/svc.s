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



.global _svc_handler
_svc_handler:
    cpsid i                     @ 1. Protege seção crítica
    
    @ --- FASE 1: SALVAR O CONTEXTO NA PILHA DA TAREFA ---
    @ O hardware já salvou o PC no LR e o CPSR no SPSR
    push {lr}                   @ context->pc   // retorno para o user mode
    mrs r12, SPSR
    push {r12}                  @ context->cpsr // modo do user

    @ Salva LR e SP do usuário (sem mudar de modo)
    stmfd sp!, {lr}^         @ context->sp e context->lr

    @ Salva R12 até R0
    sub sp, sp, #52
    stmia sp, {r0-r12}^         @ context->r0 até context->r12

    @ --- FASE 2: ATUALIZAR TCB E CHAMAR DISPATCHER ---
    ldr r4, =current_task       
    ldr r5, [r4]
    str sp, [r5, #4]            @ tcb->sp = sp atual

    @ r0-r3 já estão com os argumentos da syscall
    bl k_svc_dispatcher

    ldr r0, =current_task
    ldr r1, [r0]
    ldr r2, [r1, #4]        // Carrega o SP do TCB
    mov sp, r2              // Carrega o SP para o SP_svc

    cps 0x1F                // Entra no System Mode
    mov sp, r2              // Altera o valor de SP_user
    cps 0x13                // Retorna ao modo Supervisor

    pop {lr}                @ 1. Tira o LR_usr (0)
    pop {r0-r12}            @ 2. Tira r0 até r12 (13 registradores)
    pop {r1}                @ 3. Tira o SPSR (0x10)
    
    msr SPSR_cxsf, r1       @ 4. Prepara o SPSR com 0x10

    ldmfd sp!, {pc}^        @ 5. Volta para User Mode e pula para task_func
