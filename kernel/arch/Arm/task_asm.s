.section .text
.global start_first_task

start_first_task:
    ldr r1, =current_task   
    ldr r0, [r1]            // Carrega endereço base da task atual
    ldmfd r0!, {sp, lr}^    // Carrega o SP_user e LR_user do TCB
    ldmfd r0!, {r1}         // Carrega o SPSR_user
    msr SPSR_cxsf, r1       // Guarda o SPSR_user
    
    

    pop {lr}                @ 1. Tira o LR_usr (0)
    pop {r0-r12}            @ 2. Tira r0 até r12 (13 registradores)
    

    ldmfd sp!, {pc}^        @ 5. Volta para User Mode e pula para task_func
