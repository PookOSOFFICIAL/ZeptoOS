// author: Antonio
unsigned char color = 0x0f;
unsigned short* vga_addr = (unsigned short*)0xb8000;
unsigned short* cursor = (unsigned short*)0xb8000;
void scroll() {
    for (int i = 0; i < 24 * 80; i++) {
        vga_addr[i] = vga_addr[i + 80];
    }
    for (int i = 24 * 80; i < 25 * 80; i++) {
        vga_addr[i] = (color << 8) | ' ';
    }
    cursor = vga_addr + (24 * 80);
}
void kcls() {
    for (int i = 0; i < 80*25; i++) {
      vga_addr[i] = (color << 8) | ' ';
    }
}
void tty_write(unsigned char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') {
            int pos = cursor - vga_addr;
            int linha_atual = pos / 80;
            if (linha_atual >= 24) {
                scroll();
                cursor = vga_addr + (24 * 80);
            } else {
                cursor = vga_addr + ((linha_atual + 1) * 80);
            }
        } else {
            *cursor++ = (color << 8) | str[i];
            if (cursor >= vga_addr + (25 * 80)) {
                scroll();
                cursor = vga_addr + (24 * 80);
            }
        }
    }
}
void kprintf(unsigned char* str) {
    tty_write(str);
}
