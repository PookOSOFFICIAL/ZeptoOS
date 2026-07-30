#include "i386/gdt.h"
const char* str = "Hello, World!\0";
unsigned char color = 0x1f;
unsigned short* vga_addr = (unsigned short*)0xb8000;
void kmain() {
    for (int i = 0; i < 80*25; i++) {
      vga_addr[i] = (color << 8) | ' ';
    }

    for (int i = 0; str[i] != '\0'; i++) {
      vga_addr[i] = (color << 8) | str[i];
    }
    init_gdt();
}

