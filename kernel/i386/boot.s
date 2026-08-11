extern kmain

section .multiboot
align 4
dd 0x1BADB002
dd 3                    ; 1<<0 | 1<<1 = 3
dd -0x1BADB005          ; -(0x1BADB002 + 3)

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
