#pragma once

#include "../lib/multiboot.h"

void pmm_init(multiboot_info_t* mb);
void* pmm_alloc(void);
void pmm_free(void* addr);
void* pmm_alloc_pages(int count);
