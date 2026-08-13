#include "io.h"
#include "vga.h"
#include "../lib/types.h"

static volatile uint32_t system_tick;
static volatile uint32_t seconds;

void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

void timer_handler(void) {
    system_tick++;
    if (system_tick >= 100) {
        system_tick = 0;
        seconds++;
    }
    outb(0x20, 0x20);
}
