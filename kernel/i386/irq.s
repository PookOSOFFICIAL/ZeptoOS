global irq0_stub
global irq1_stub
extern schedule
extern keyboard_handler

irq0_stub:
    pusha
    push esp
    call schedule
    mov esp, eax
    mov al, 0x20
    out 0x20, al
    popa
    iretd

irq1_stub:
    pusha
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popa
    iretd
