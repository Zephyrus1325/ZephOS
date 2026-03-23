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

/*




 */
_svc_handler:
    cpsid i                 // Desativa interrupções
    
    ldr r12, =current_task
    ldr sp, [r12]               // task sp = *current_task (current_task é um *tcb_t)
    add sp, sp, #64             // Vai para o final do context
    stmda sp!, {lr}             // Salva o pc da tarefa
    
    stmda sp!, {r0-r12}         // Salva todos os registradores do usuario

    mrs r12, SPSR          // Carrega o SPSR_user
    stmda sp!, {r12}            // Salva o SPSR_user
    cps #0x1F                   // Entra no System Mode
    mov r11, sp
    mov r12, lr              
    cps #0x13                   // Volta para o modo SVC
    stmda sp!, {r11, r12}       // Guarda os sp e lr de volta

    ldr r11, =_sp_svc
    ldr sp, [r11]               // Carrega o SP do kernel

    bl k_svc_dispatcher         

    ldr r1, =current_task   //
    ldr r0, [r1]            // task r0 = *current_task (current_task é um *tcb_t)

    
    ldmfd r0, {sp, lr}^     // Carrega e guarda o SP_user e LR_user
    add r0, r0, #8          // Bleh
    ldmfd r0!, {r1}         // Carrega o SPSR_user
    msr SPSR_cxsf, r1       // Guarda o SPSR_user

    ldr r1, =_sp_svc  
    str sp, [r1]            // Armazena o valor do SP do kernel para uso futuro
    mov sp, r0              // Coloca o endereço da task no SP

    ldmfd sp!, {r0-r12}     // Carrega os registradores r0-r12 (13 registradores)


    ldmfd sp!, {pc}^            // Retorna para a task que chamou, ou para a task que AAAAA
