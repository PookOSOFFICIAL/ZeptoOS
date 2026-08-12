#include "io.h"
#include "../drv/vga.h"
void pic_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFE);
    outb(0xA1, 0xFF);
}
volatile uint32_t system_tick = 0;
volatile uint32_t seconds = 0;

void timer_handler() {
    system_tick++;
    if (system_tick >= 20) {
        system_tick = 0;
        seconds++;
	kprintf("Tick\n");
    }
    outb(0x20, 0x20);  // coloca isso se nao o kernel explode
}
