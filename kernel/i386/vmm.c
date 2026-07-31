#include "../lib/types.h"
#include "../mm/pmm.h"
#define PAGE_SIZE 4096
#define PAGE_PRESENT 1
#define PAGE_WRITE 2

uint32_t* page_directory_ptr;

void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    uint32_t pd_index = virt_addr / (1024 * 4096);
    uint32_t pt_index = (virt_addr / 4096) % 1024;

    if (!(page_directory_ptr[pd_index] & PAGE_PRESENT)) {
        uint32_t pt_phys = (uint32_t)pmm_alloc();
        uint32_t* pt_virt = (uint32_t*)pt_phys;
        for(int i = 0; i < 1024; i++) pt_virt[i] = 0;
        page_directory_ptr[pd_index] = pt_phys | flags | PAGE_PRESENT;
    }

    uint32_t pt_phys = page_directory_ptr[pd_index] & 0xFFFFF000;
    uint32_t* page_table_virt = (uint32_t*)pt_phys;
    page_table_virt[pt_index] = (phys_addr & 0xFFFFF000) | flags | PAGE_PRESENT;
}

void enable_paging() {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

void load_page_directory(uint32_t phys_addr) {
    asm volatile("mov %0, %%cr3" : : "r"(phys_addr));
}

void init_paging() {
    uint32_t pd_phys = (uint32_t)pmm_alloc();
    page_directory_ptr = (uint32_t*)pd_phys;
    for(int i = 0; i < 1024; i++) page_directory_ptr[i] = 0;
    uint32_t pt_phys = (uint32_t)pmm_alloc();
    uint32_t* pt_virt = (uint32_t*)pt_phys;
    for(int i = 0; i < 1024; i++) pt_virt[i] = 0;
    for(uint32_t i = 0; i < 1024; i++) {
        pt_virt[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_WRITE;
    }
    page_directory_ptr[0] = pt_phys | PAGE_PRESENT | PAGE_WRITE;
    load_page_directory(pd_phys);
    enable_paging();
}
