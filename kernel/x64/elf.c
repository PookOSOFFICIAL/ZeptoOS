#include "elf.h"

uintptr_t elf_load(uint8_t* elf_data) {
    struct elf64_hdr* header = (struct elf64_hdr*)elf_data;
    if (header->e_ident != ELF_MAGIC || header->e_class != 2 || header->e_machine != 0x3E) {
        return 0;
    }
    struct elf64_phdr* program_headers = (struct elf64_phdr*)(elf_data + header->e_phoff);
    for (uint16_t i = 0; i < header->e_phnum; i++) {
        if (program_headers[i].p_type == 1) {
            uint8_t* destination = (uint8_t*)(uintptr_t)program_headers[i].p_vaddr;
            uint8_t* source = elf_data + program_headers[i].p_offset;
            for (uint64_t j = 0; j < program_headers[i].p_filesz; j++) {
                destination[j] = source[j];
            }
            for (uint64_t j = program_headers[i].p_filesz; j < program_headers[i].p_memsz; j++) {
                destination[j] = 0;
            }
        }
    }
    return (uintptr_t)header->e_entry;
}
