#pragma once

#include "../lib/types.h"

#define PAGE_SIZE 4096
#define PAGE_PRESENT 1
#define PAGE_WRITE 2
#define PAGE_USER 4

typedef struct page_directory {
    uint64_t entries[512];
} page_directory_t;

extern page_directory_t* current_pd;
extern page_directory_t* kernel_pd;

void init_paging(void);
page_directory_t* vmm_create_address_space(void);
void vmm_switch_to(page_directory_t* pd);
void vmm_map_page(page_directory_t* pd, uintptr_t virt, uintptr_t phys, uint64_t flags);
void vmm_unmap_page(page_directory_t* pd, uintptr_t virt);
uintptr_t vmm_get_phys_addr(page_directory_t* pd, uintptr_t virt);
void vmm_clone_address_space(page_directory_t* src, page_directory_t* dst);
