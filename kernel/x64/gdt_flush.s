bits 64

global gdt_flush
global tss_flush

section .text
gdt_flush:
    lgdt [rdi]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    push qword 0x08
    lea rax, [rel .flush]
    push rax
    retfq
.flush:
    ret

tss_flush:
    mov ax, 0x28
    ltr ax
    ret
