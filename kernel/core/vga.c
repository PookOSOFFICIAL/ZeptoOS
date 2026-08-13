#include "vga.h"
#include "serial.h"
#include "../lib/types.h"

static uint8_t color = 0x0F;
static volatile uint16_t* vga_addr = (volatile uint16_t*)0xB8000;
static volatile uint16_t* cursor = (volatile uint16_t*)0xB8000;

void scroll(void) {
    for (int i = 0; i < 24 * 80; i++) {
        vga_addr[i] = vga_addr[i + 80];
    }
    for (int i = 24 * 80; i < 25 * 80; i++) {
        vga_addr[i] = ((uint16_t)color << 8) | ' ';
    }
    cursor = vga_addr + (24 * 80);
}

void kcls(void) {
    for (int i = 0; i < 80 * 25; i++) {
        vga_addr[i] = ((uint16_t)color << 8) | ' ';
    }
    cursor = vga_addr;
}

void tty_write(const char* str) {
    serial_printf(str);
    while (*str) {
        char c = *str++;
        if (c == '\n') {
            uintptr_t pos = (uintptr_t)(cursor - vga_addr);
            uintptr_t row = pos / 80;
            if (row >= 24) {
                scroll();
            } else {
                cursor = vga_addr + ((row + 1) * 80);
            }
        } else {
            *cursor++ = ((uint16_t)color << 8) | (uint8_t)c;
            if (cursor >= vga_addr + (25 * 80)) {
                scroll();
            }
        }
    }
}

void kprintf(const char* str) {
    tty_write(str);
}
