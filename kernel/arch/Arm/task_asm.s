.section .text
//.global start_first_task

start_first_task:
    ldr r0, =current_task
    ldr r1, [r0]
    ldr sp, [r1, #4]        @ Carrega o SP do TCB

    pop {lr}                @ 1. Tira o LR_usr (0)
    pop {r0-r12}            @ 2. Tira r0 até r12 (13 registradores)
    pop {r1}                @ 3. Tira o SPSR (0x10)
    
    msr SPSR_cxsf, r1       @ 4. Prepara o SPSR com 0x10

    ldmfd sp!, {pc}^        @ 5. Volta para User Mode e pula para task_func
