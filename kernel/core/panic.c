#include "panic.h"
#include "vga.h"

void panic(const char* msg) {
    kprintf("PANIC ");
    kprintf(msg);
    kprintf("\n");
    asm volatile("cli");
    for (;;) {
        asm volatile("hlt");
    }
}
