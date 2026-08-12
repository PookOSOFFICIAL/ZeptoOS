#include "i386/gdt.h"
#include "i386/idt.h"
#include "drv/vga.h"
#include "i386/pic.h"
#include "i386/pit.h"
#include "mm/pmm.h"
#include "i386/vmm.h"
#include "scheduler.h"
#include "drv/serial.h"

struct multiboot_info* mb_info;

void task_entry() {
    uint32_t pid = get_current_pid();
    while (1) {
	kprintf("Task: ");
        print_pid(pid);
	kprintf("\n");
        for (volatile int i = 0; i < 2000000; i++);
    }
}

void kmain(uint32_t magic, struct multiboot_info* info) {
    mb_info = info;
    init_gdt();
    idt_install();
    pic_remap();
    pit_init();
    pmm_init(mb_info);
    init_paging();
    serial_init();

    scheduler_init();
    for (int i = 0; i < 10; i++) {
        task_create(task_entry);
    }

    asm volatile("sti");

    while (1) {
        asm volatile("hlt");
    }
}
