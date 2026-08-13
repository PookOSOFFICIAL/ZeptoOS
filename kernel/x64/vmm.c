#include "vmm.h"
#include "../mm/pmm.h"

page_directory_t* current_pd;
page_directory_t* kernel_pd;

static void clear_page(void* page) {
    uint8_t* bytes = (uint8_t*)page;
    for (int i = 0; i < PAGE_SIZE; i++) {
        bytes[i] = 0;
    }
}

static uint64_t* next_table(uint64_t* table, unsigned int index, int create) {
    if (!(table[index] & PAGE_PRESENT)) {
        if (!create) {
            return NULL;
        }
        uint64_t* page = (uint64_t*)pmm_alloc();
        clear_page(page);
        table[index] = (uint64_t)(uintptr_t)page | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    return (uint64_t*)(uintptr_t)(table[index] & 0x000FFFFFFFFFF000ULL);
}

static uint64_t* page_table(page_directory_t* root, uintptr_t virt, int create) {
    unsigned int pml4_index = (unsigned int)((virt >> 39) & 0x1FF);
    unsigned int pdpt_index = (unsigned int)((virt >> 30) & 0x1FF);
    unsigned int pd_index = (unsigned int)((virt >> 21) & 0x1FF);
    uint64_t* pdpt = next_table(root->entries, pml4_index, create);
    if (!pdpt) {
        return NULL;
    }
    uint64_t* pd = next_table(pdpt, pdpt_index, create);
    if (!pd) {
        return NULL;
    }
    return next_table(pd, pd_index, create);
}

void init_paging(void) {
    uintptr_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    kernel_pd = (page_directory_t*)cr3;
    current_pd = kernel_pd;
}

page_directory_t* vmm_create_address_space(void) {
    page_directory_t* page_directory = (page_directory_t*)pmm_alloc();
    clear_page(page_directory);
    for (int i = 0; i < 512; i++) {
        page_directory->entries[i] = kernel_pd->entries[i];
    }
    return page_directory;
}

void vmm_switch_to(page_directory_t* page_directory) {
    current_pd = page_directory;
    asm volatile("mov %0, %%cr3" : : "r"((uintptr_t)page_directory) : "memory");
}

void vmm_map_page(page_directory_t* page_directory, uintptr_t virt, uintptr_t phys, uint64_t flags) {
    uint64_t* table = page_table(page_directory, virt, 1);
    if (!table) {
        return;
    }
    unsigned int index = (unsigned int)((virt >> 12) & 0x1FF);
    table[index] = (phys & 0x000FFFFFFFFFF000ULL) | flags | PAGE_PRESENT;
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_unmap_page(page_directory_t* page_directory, uintptr_t virt) {
    uint64_t* table = page_table(page_directory, virt, 0);
    if (!table) {
        return;
    }
    unsigned int index = (unsigned int)((virt >> 12) & 0x1FF);
    table[index] = 0;
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uintptr_t vmm_get_phys_addr(page_directory_t* page_directory, uintptr_t virt) {
    unsigned int pml4_index = (unsigned int)((virt >> 39) & 0x1FF);
    unsigned int pdpt_index = (unsigned int)((virt >> 30) & 0x1FF);
    unsigned int pd_index = (unsigned int)((virt >> 21) & 0x1FF);
    unsigned int pt_index = (unsigned int)((virt >> 12) & 0x1FF);
    uint64_t pml4e = page_directory->entries[pml4_index];
    if (!(pml4e & PAGE_PRESENT)) {
        return 0;
    }
    uint64_t* pdpt = (uint64_t*)(uintptr_t)(pml4e & 0x000FFFFFFFFFF000ULL);
    uint64_t pdpte = pdpt[pdpt_index];
    if (!(pdpte & PAGE_PRESENT)) {
        return 0;
    }
    uint64_t* pd = (uint64_t*)(uintptr_t)(pdpte & 0x000FFFFFFFFFF000ULL);
    uint64_t pde = pd[pd_index];
    if (!(pde & PAGE_PRESENT)) {
        return 0;
    }
    if (pde & 0x80) {
        return (uintptr_t)((pde & 0x000FFFFFFFE00000ULL) | (virt & 0x1FFFFF));
    }
    uint64_t* pt = (uint64_t*)(uintptr_t)(pde & 0x000FFFFFFFFFF000ULL);
    uint64_t pte = pt[pt_index];
    if (!(pte & PAGE_PRESENT)) {
        return 0;
    }
    return (uintptr_t)((pte & 0x000FFFFFFFFFF000ULL) | (virt & 0xFFF));
}

void vmm_clone_address_space(page_directory_t* source, page_directory_t* destination) {
    for (int i = 0; i < 512; i++) {
        destination->entries[i] = source->entries[i];
    }
}
