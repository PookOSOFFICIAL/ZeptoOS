global irq0_stub
global irq1_stub

extern schedule
extern timer_handler
extern keyboard_handler

section .text
irq0_stub:
    pusha
    call timer_handler
    push esp
    call schedule
    mov esp, eax
    popa
    iretd

irq1_stub:
    pusha
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popa
    iretd
