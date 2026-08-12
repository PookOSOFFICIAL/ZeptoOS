#include "panic.h"

void panic(char* msg) {
    kprintf("PANIC ");
    kprintf(msg);
    kprintf("\n");
    while(1) {
	asm volatile("cli; hlt");
    }
}
