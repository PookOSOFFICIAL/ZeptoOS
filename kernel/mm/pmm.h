#pragma once
void pmm_init();
void* pmm_alloc();
void pmm_free(void* addr);
void* pmm_alloc_pages(int count);
