#include "core/arch.h"
#include "core/vga.h"
#include "core/keyboard.h"
#include "core/pic.h"
#include "core/pit.h"
#include "core/scheduler.h"
#include "core/serial.h"
#include "core/pata.h"
#include "mm/pmm.h"
#include "fs/vfs.h"
#include "fs/tar.h"
#include "fs/ext2.h"
#include "lib/multiboot.h"
#include "lib/types.h"

multiboot_info_t* mb_info;

void kmain(uint32_t magic, multiboot_info_t* info) {
    mb_info = info;
    serial_init();
    kcls();
#ifdef __x86_64__
    kprintf("ZeptoOS x86_64 boot\n");
#else
    kprintf("ZeptoOS i386 boot\n");
#endif
    if (magic != 0x2BADB002) {
        kprintf("Invalid Multiboot magic\n");
        for (;;) {
            asm volatile("hlt");
        }
    }
    init_gdt();
    idt_install();
    pic_remap();
    pit_init();
    pmm_init(mb_info);
    init_paging();
    vfs_init();
    pata_init();
    keyboard_init();
    kprintf("Core initialized\n");
    if (info->mods_count > 0) {
        multiboot_module_t* modules = (multiboot_module_t*)(uintptr_t)info->mods_addr;
        struct vfs_node* tar_root = tar_parse(modules->mod_start);
        vfs_mount("/", tar_root);
        struct vfs_node* init_node = vfs_finddir(tar_root, "init");
        if (init_node && init_node->read) {
            uint32_t size = init_node->length;
            uint8_t* elf_buffer = (uint8_t*)0x400000;
            init_node->read(init_node, 0, size, elf_buffer);
            uintptr_t entry = elf_load(elf_buffer);
            if (entry) {
                scheduler_init();
                task_create_user((void (*)(void))entry);
                kprintf("User init loaded\n");
            } else {
                kprintf("User init load failed\n");
            }
        } else {
            kprintf("User init missing\n");
        }
    } else {
        kprintf("Initrd missing\n");
    }
    kprintf("Interrupts enabled\n");
    asm volatile("sti");
    for (;;) {
        asm volatile("hlt");
    }
}
