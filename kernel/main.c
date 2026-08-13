#include "i386/gdt.h"
#include "i386/idt.h"
#include "drv/vga.h"
#include "drv/keyboard.h"
#include "i386/pic.h"
#include "i386/pit.h"
#include "mm/pmm.h"
#include "i386/vmm.h"
#include "scheduler.h"
#include "drv/serial.h"
#include "drv/pata.h"
#include "fs/vfs.h"
#include "fs/tar.h"
#include "fs/ext2.h"
#include "i386/elf.h"
#include "lib/multiboot.h"

multiboot_info_t* mb_info;

void kmain(uint32_t magic, multiboot_info_t* info) {
    mb_info = info;
    init_gdt();
    idt_install();
    pic_remap();
    pit_init();
    pmm_init(mb_info);
    init_paging();
    serial_init();

    vfs_init();
    pata_init();
    keyboard_init();

    if (info->mods_count > 0) {
        multiboot_module_t* mod = (multiboot_module_t*)info->mods_addr;
        struct vfs_node* tar_root = tar_parse(mod->mod_start);
        vfs_mount("/", tar_root);

        struct vfs_node* init_node = vfs_finddir(tar_root, "init");
        if (init_node && init_node->read) {
            uint32_t size = init_node->length;
            uint8_t* elf_buf = (uint8_t*)0x400000;
            init_node->read(init_node, 0, size, elf_buf);
            uint32_t entry = elf_load(elf_buf);
            if (entry) {
                scheduler_init();
                task_create_user((void (*)())entry);
            }
        }
    }

    asm volatile("sti");

    while (1) {
        asm volatile("hlt");
    }
}
