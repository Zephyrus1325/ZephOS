.section .text
.global _irq_handler

_irq_handler:
    cpsid i                 // Desativa interrupções
    
    push {r0}               // COloca R0 e R1 no stack IRQ
    mov r0, lr              // Salva o PC do user
    sub r0, r0, #4          // lr - 4
    cps 0x1F                // Vai pra System mode
    push {r0}               // Salva PC no stack do user
    
    cps 0x12               // Volta pra IRQ
    pop {r0}                // Pega r0 do stack IRQ
    cps 0x1F                // Vai pra System mode de novo
    push {r0-r12}            // Guarda isso tudo no stack do usuario

    mrs r11, SPSR
    push {r11, lr}           // Salva CPSR do usuario e LR

    ldr r11, =current_task   // Carrega o TCB da task
    ldr r12, [r11]
    add r12, r12, #12         // Pega o endereço do stack_base
    str sp, [r12]            // salva o valor atual do stack_user
    
    cps 0x12               // Volta pra IRQ

    bl k_irq_handler        // Realiza a subrotina do IRQ

    cps 0x1F                // Vai pra system mode
    
    ldr r0, =current_task
    ldr r1, [r0]    
    add r1, r1, #12         // Pega o endereço do stack_base
    ldr sp, [r1]            // Carrega o stack novo

    pop {r0, lr}            // Carrega o CPSR e LR
    msr SPSR, r0            // Salva o CPSR novo
    
    pop {r0-r12}            // Carrega os registradores r0-r12 de volta
    
    ldmfd sp!, {pc}^        // Retorna para a task principal e coloca SPSR em CPSR
