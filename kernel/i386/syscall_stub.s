global syscall_stub
extern syscall_handler

syscall_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call syscall_handler
    add esp, 4
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iretd
