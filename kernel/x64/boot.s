bits 32

section .multiboot
align 4
dd 0x1BADB002
dd 3
dd -0x1BADB005

section .bss
align 4096
pml4_table:
resb 4096
align 4096
pdpt_table:
resb 4096
align 4096
pd_table:
resb 4096
align 16
stack_bottom:
resb 16384
stack_top:
boot_magic:
resd 1
boot_info:
resd 1

section .rodata
align 8
gdt64:
dq 0
dq 0x00AF9A000000FFFF
dq 0x00AF92000000FFFF
gdt64_end:
gdt64_pointer:
dw gdt64_end - gdt64 - 1
dd gdt64

section .text
global _start
extern kmain

_start:
    mov esp, stack_top
    mov [boot_magic], eax
    mov [boot_info], ebx
    call setup_page_tables
    mov eax, cr4
    or eax, 0x30
    mov cr4, eax
    mov eax, pml4_table
    mov cr3, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    lgdt [gdt64_pointer]
    jmp 0x08:long_mode_start

setup_page_tables:
    mov eax, pdpt_table
    or eax, 0x07
    mov [pml4_table], eax
    mov dword [pml4_table + 4], 0
    mov eax, pd_table
    or eax, 0x07
    mov [pdpt_table], eax
    mov dword [pdpt_table + 4], 0
    xor ecx, ecx
.map_pages:
    mov eax, ecx
    shl eax, 21
    or eax, 0x87
    mov [pd_table + ecx * 8], eax
    mov dword [pd_table + ecx * 8 + 4], 0
    inc ecx
    cmp ecx, 512
    jne .map_pages
    ret

bits 64
long_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor eax, eax
    mov fs, ax
    mov gs, ax
    mov rsp, stack_top
    mov edi, dword [boot_magic]
    mov esi, dword [boot_info]
    call kmain
    cli
.halt:
    hlt
    jmp .halt
