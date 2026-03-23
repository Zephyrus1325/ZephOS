.section .text
.global _irq_handler

_irq_handler:
    cpsid i                 // Desativa interrupções
    
    ldr r12, =current_task
    ldr sp, [r12]               // task sp = *current_task (current_task é um *tcb_t)
    add sp, sp, #64             // Vai para o final do context
    stmda sp!, {lr}             // Salva o pc da tarefa
    
    stmda sp!, {r0-r12}         // Salva todos os registradores do usuario

    mrs r12, SPSR               // Carrega o SPSR_user
    stmda sp!, {r12}            // Salva o SPSR_user
    cps #0x1F                   // Entra no System Mode
    mov r11, sp
    mov r12, lr              
    cps #0x12                   // Volta para o modo IRQ
    stmda sp!, {r11, r12}       // Guarda os sp e lr de volta

    ldr r11, =_sp_irq
    ldr sp, [r11]               // Carrega o SP do kernel

    bl k_irq_handler         

    ldr r1, =current_task   //
    ldr r0, [r1]            // task r0 = *current_task (current_task é um *tcb_t)

    
    ldmfd r0, {sp, lr}^     // Carrega e guarda o SP_user e LR_user
    add r0, r0, #8          // Bleh
    ldmfd r0!, {r1}         // Carrega o SPSR_user
    msr SPSR_cxsf, r1       // Guarda o SPSR_user

    ldr r1, =_sp_irq  
    str sp, [r1]            // Armazena o valor do SP do kernel para uso futuro
    mov sp, r0              // Coloca o endereço da task no SP

    ldmfd sp!, {r0-r12}     // Carrega os registradores r0-r12 (13 registradores)


    ldmfd sp!, {pc}^            // Retorna para a task que chamou, ou para a task que AAAAA
