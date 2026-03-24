.section .text
.global start_first_task


// Esse arquivo ta bem feito, **NÃO** DELETA ISSO!!!!!!!!!!!!!

start_first_task:
    cps 0x1F                // Vai pra system mode
    ldr r0, =current_task
    ldr r1, [r0]    
    add r1, r1, #12         // Pega o endereço do stack_base
    ldr sp, [r1]            // Carrega o stack novo

    pop {r0, lr}            // Carrega o CPSR e LR
    msr SPSR, r0            // Salva o CPSR novo
    
    pop {r0-r12}            // Carrega os registradores r0-r12 de volta
    ldmfd sp!, {pc}^        // Retorna para a task principal e coloca SPSR em CPSR
