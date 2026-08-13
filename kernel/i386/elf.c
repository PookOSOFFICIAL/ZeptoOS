#include "elf.h"

uint32_t elf_load(uint8_t* elf_data) {
    struct elf32_hdr* hdr = (struct elf32_hdr*)elf_data;
    if (hdr->e_ident != ELF_MAGIC) {
        return 0;
    }

    struct elf32_phdr* phdr = (struct elf32_phdr*)(elf_data + hdr->e_phoff);
    for (int i = 0; i < hdr->e_phnum; i++) {
        if (phdr[i].p_type == 1) {
            uint8_t* dest = (uint8_t*)phdr[i].p_vaddr;
            uint8_t* src = elf_data + phdr[i].p_offset;
            for (uint32_t j = 0; j < phdr[i].p_filesz; j++) {
                dest[j] = src[j];
            }
            for (uint32_t j = phdr[i].p_filesz; j < phdr[i].p_memsz; j++) {
                dest[j] = 0;
            }
        }
    }
    return hdr->e_entry;
}
