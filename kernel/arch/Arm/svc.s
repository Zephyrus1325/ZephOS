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
    cpsid i                 // Desativa interrupções
    
    push {r0}               // COloca R0 e R1 no stack SVC
    mov r0, lr              // Salva o PC do user
    cps 0x1F                // Vai pra System mode
    push {r0}               // Salva PC no stack do user
    
    cps 0x13                // Volta pra SVC
    pop {r0}                // Pega r0 do stack SVC
    cps 0x1F                // Vai pra System mode de novo
    push {r0-r12}            // Guarda isso tudo no stack do usuario

    mrs r8, SPSR
    push {r8, lr}           // Salva CPSR do usuario e LR

    ldr r8, =current_task   // Carrega o TCB da task
    ldr r9, [r8]
    add r9, r9, #12         // Pega o endereço do stack_base
    str sp, [r9]            // salva o valor atual do stack_user
    
    cps 0x13               // Volta pra SVC

    bl k_svc_dispatcher     // Realiza a subrotina do SVC

    cps 0x1F                // Vai pra system mode
    
    ldr r0, =current_task
    ldr r1, [r0]    
    add r1, r1, #12         // Pega o endereço do stack_base
    ldr sp, [r1]            // Carrega o stack novo

    pop {r0, lr}            // Carrega o CPSR e LR
    msr SPSR, r0            // Salva o CPSR novo
    
    pop {r0-r12}            // Carrega os registradores r0-r12 de volta
    
    ldmfd sp!, {pc}^        // Retorna para a task principal e coloca SPSR em CPSR
