global irq0_stub
extern schedule

irq0_stub:
    pusha
    
    push esp
    call schedule
    mov esp, eax
    
    mov al, 0x20
    out 0x20, al
    
    popa
    iretd
