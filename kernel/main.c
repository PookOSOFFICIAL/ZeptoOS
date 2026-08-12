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

void user_hello_task() {
    char *msg = "Hello World from Ring 3!\n";
    int len = 25;
    asm volatile(
        "mov $4, %%eax\n"
        "mov $1, %%ebx\n"
        "mov %0, %%ecx\n"
        "mov %1, %%edx\n"
        "int $0x80\n"
        "mov $1, %%eax\n"
        "xor %%ebx, %%ebx\n"
        "int $0x80\n"
        :
        : "r"(msg), "r"(len)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    while (1);
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
    task_create_user(user_hello_task);

    asm volatile("sti");

    while (1) {
        asm volatile("hlt");
    }
}
