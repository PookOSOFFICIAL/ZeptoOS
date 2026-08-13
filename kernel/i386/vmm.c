#include "vmm.h"
#include "../mm/pmm.h"
#include "../i386/panic.h"

page_directory_t* current_pd;
page_directory_t* kernel_pd;

static uint32_t* get_page_table(page_directory_t* pd, uint32_t virt, int create) {
    uint32_t pd_idx = virt >> 22;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    
    if (!(pd->entries[pd_idx] & PAGE_PRESENT)) {
        if (!create) return NULL;
        uint32_t pt_phys = (uint32_t)pmm_alloc();
        uint32_t* pt_virt = (uint32_t*)pt_phys;
        for (int i = 0; i < 1024; i++) pt_virt[i] = 0;
        pd->entries[pd_idx] = pt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    uint32_t pt_phys = pd->entries[pd_idx] & 0xFFFFF000;
    return (uint32_t*)pt_phys;
}

void init_paging(void) {
    kernel_pd = (page_directory_t*)pmm_alloc();
    for (int i = 0; i < 1024; i++) kernel_pd->entries[i] = 0;
    
    for (uint32_t addr = 0; addr < 0x1000000; addr += 0x1000) {
        vmm_map_page(kernel_pd, addr, addr, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
    
    current_pd = kernel_pd;
    asm volatile("mov %0, %%cr3" : : "r"((uint32_t)kernel_pd));
    
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

page_directory_t* vmm_create_address_space(void) {
    page_directory_t* pd = (page_directory_t*)pmm_alloc();
    for (int i = 0; i < 1024; i++) pd->entries[i] = 0;
    for (int i = 0; i < 1024; i++) {
        pd->entries[i] = kernel_pd->entries[i];
    }
    return pd;
}

void vmm_switch_to(page_directory_t* pd) {
    current_pd = pd;
    asm volatile("mov %0, %%cr3" : : "r"((uint32_t)pd));
}

void vmm_map_page(page_directory_t* pd, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t* pt = get_page_table(pd, virt, 1);
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    pt[pt_idx] = (phys & 0xFFFFF000) | flags | PAGE_PRESENT;
}

void vmm_unmap_page(page_directory_t* pd, uint32_t virt) {
    uint32_t* pt = get_page_table(pd, virt, 0);
    if (!pt) return;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    pt[pt_idx] = 0;
}

uint32_t vmm_get_phys_addr(page_directory_t* pd, uint32_t virt) {
    uint32_t* pt = get_page_table(pd, virt, 0);
    if (!pt) return 0;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;
    return pt[pt_idx] & 0xFFFFF000;
}

void vmm_clone_address_space(page_directory_t* src, page_directory_t* dst) {
    for (int i = 0; i < 1024; i++) {
        if (src->entries[i] & PAGE_PRESENT) {
            uint32_t src_pt_phys = src->entries[i] & 0xFFFFF000;
            uint32_t* src_pt = (uint32_t*)src_pt_phys;
            uint32_t dst_pt_phys = (uint32_t)pmm_alloc();
            uint32_t* dst_pt = (uint32_t*)dst_pt_phys;
            for (int j = 0; j < 1024; j++) {
                if (src_pt[j] & PAGE_PRESENT) {
                    uint32_t phys = src_pt[j] & 0xFFFFF000;
                    uint32_t flags = src_pt[j] & ~0xFFFFF000;
                    dst_pt[j] = phys | flags;
                } else {
                    dst_pt[j] = 0;
                }
            }
            dst->entries[i] = dst_pt_phys | (src->entries[i] & ~0xFFFFF000);
        }
    }
}
