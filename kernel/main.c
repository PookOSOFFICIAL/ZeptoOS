#include "i386/gdt.h"
#include "i386/idt.h"
#include "drv/vga.h"
#include "i386/pic.h"
#include "i386/pit.h"
#include "mm/pmm.h"
#include "i386/vmm.h"

void kmain() {
    kprintf("GDT Initing...\n");
    init_gdt();
    kprintf("GDT OK\n");

    kprintf("IDT Initing...\n");
    idt_install();
    pic_remap();
    pit_init();
    kprintf("IDT OK\n");

    pmm_init();
    init_paging();
    asm volatile("sti");

    while (1) {
	asm volatile("sti; hlt");
    }
}

