#include "serial.h"
#include "../i386/io.h"

void serial_init() {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

int is_transmit_empty() {
    return inb(0x3F8 + 5) & 0x20;
}

void serial_write(char c) {
    while (is_transmit_empty() == 0);
    outb(0x3F8, c);
}

void serial_printf(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        serial_write(str[i]);
    }
}
