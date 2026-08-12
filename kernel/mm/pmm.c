#include "../lib/multiboot.h"
#include "../lib/types.h"
#include "../i386/panic.h"

extern multiboot_info_t* mb_info;
#define BIT_SET(a, i) ((a)[(i) / 8] |= (1 << ((i) % 8)))
#define BIT_TEST(a, i) ((a)[(i) / 8] & (1 << ((i) % 8)))
#define PMM_MAX_PAGES (4 * 1024 * 1024 / 4096)
uint8_t pmm_bitmap[PMM_MAX_PAGES / 8];

void pmm_init(multiboot_info_t* mb) {
    if (!(mb->flags & (1 << 6))) {
        return;
    }

    for (int i = 0; i < PMM_MAX_PAGES / 8; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mb->mmap_addr;
    uint32_t mmap_end = mb->mmap_addr + mb->mmap_length;

    while ((uint32_t)mmap < mmap_end) {
        if (mmap->type == 1) {
            uint64_t base = mmap->base_addr;
            uint64_t length = mmap->length;

            uint32_t start_page = (base + 0xFFF) / 4096;
            uint32_t end_page = (base + length) / 4096;

            for (uint32_t i = start_page; i < end_page && i < PMM_MAX_PAGES; i++) {
                pmm_bitmap[i / 8] &= ~(1 << (i % 8));
            }
        }
        mmap = (multiboot_mmap_entry_t*)((uint32_t)mmap + mmap->size + 4);
    }

    for (uint32_t i = 0x10000; i < 0x100000 / 4096; i++) {
        pmm_bitmap[i / 8] |= (1 << (i % 8));
    }

    for (uint32_t i = (uint32_t)mb_info / 4096; i < ((uint32_t)mb_info + sizeof(multiboot_info_t) + 4095) / 4096; i++) {
        pmm_bitmap[i / 8] |= (1 << (i % 8));
    }
}

void* pmm_alloc() {
    for (int i = 0; i < PMM_MAX_PAGES; i++) {
        if (!(pmm_bitmap[i / 8] & (1 << (i % 8)))) {
            pmm_bitmap[i / 8] |= (1 << (i % 8));
            return (void*)(i * 4096);
        }
    }
    panic("Out of Memory");
    return NULL;
}

void pmm_free(void* addr) {
    int page = (uint32_t)addr / 4096;
    pmm_bitmap[page / 8] &= ~(1 << (page % 8));
}

void* pmm_alloc_pages(int count) {
    int start = -1;
    int found = 0;
    for (int i = 0; i < PMM_MAX_PAGES; i++) {
        if (!BIT_TEST(pmm_bitmap, i)) {
            if (start == -1) start = i;
            found++;
            if (found == count) {
                for (int j = start; j < start + count; j++) {
                    BIT_SET(pmm_bitmap, j);
                }
                return (void*)(start * 4096);
            }
        } else {
            start = -1;
            found = 0;
        }
    }
    panic("Out of Memory");
    return NULL;
}
