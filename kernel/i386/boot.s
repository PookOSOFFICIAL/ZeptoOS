extern kmain

section .multiboot
align 4
dd 0x1BADB002
dd 3
dd -0x1BADB005

section .bss
align 16
stack_bottom:
resb 16384
stack_top:

section .text
global _start
_start:
    mov esp, stack_top
    push ebx
    push eax
    call kmain
    cli
.halt:
    hlt
    jmp .halt
