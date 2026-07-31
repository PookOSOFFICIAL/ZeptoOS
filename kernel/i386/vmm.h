#ifndef VMM_H
#define VMM_H
#include "../lib/types.h"
#define PAGE_SIZE 4096
#define PAGE_PRESENT 1
#define PAGE_WRITE 2
void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void enable_paging(void);
void load_page_directory(uint32_t phys_addr);
void init_paging(void);
#endif
