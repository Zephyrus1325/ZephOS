.section .text
.global start_first_task


// Esse arquivo ta bem feito, **NÃO** DELETA ISSO!!!!!!!!!!!!!

start_first_task:
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

    ldmfd sp!, {pc}^        // Carrega e inicia a tarefa
