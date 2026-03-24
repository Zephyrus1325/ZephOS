.section .text
.global _data_abort_handler
.global _handler_undef

_data_abort_handler:
_handler_undef:
    cpsid i
    // 1. Recuperar o endereço da instrução que falhou (LR - 8)
    SUB LR, LR, #8
    
    // 2. Ler o endereço que causou a falha (DFAR)
    MRC p15, 0, R0, c6, c0, 0  // R0 = Endereço da falha
    
    // 3. Ler o motivo da falha (DFSR)
    MRC p15, 0, R1, c5, c0, 0  // R1 = Status/Motivo
    
    // 4. Passar para uma função C para imprimir bonitinho
    // O LR (instrução) vai como 3º argumento (R2)
    MOV R2, LR
    B k_panic_data_abort
    b .
