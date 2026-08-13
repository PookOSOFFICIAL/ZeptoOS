#include "../lib/multiboot.h"
#include "../lib/types.h"
#include "../core/panic.h"

extern uint8_t _kernel_end;

#define BIT_SET(a, i) ((a)[(i) / 8] |= (1U << ((i) % 8)))
#define BIT_TEST(a, i) ((a)[(i) / 8] & (1U << ((i) % 8)))
#define PMM_MAX_PAGES (16 * 1024 * 1024 / 4096)

static uint8_t pmm_bitmap[PMM_MAX_PAGES / 8];

void pmm_init(multiboot_info_t* mb) {
    for (int i = 0; i < PMM_MAX_PAGES / 8; i++) {
        pmm_bitmap[i] = 0xFF;
    }
    if (mb->flags & (1U << 6)) {
        multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)(uintptr_t)mb->mmap_addr;
        uintptr_t mmap_end = (uintptr_t)mb->mmap_addr + mb->mmap_length;
        while ((uintptr_t)mmap < mmap_end) {
            if (mmap->type == 1) {
                uint64_t start_page = mmap->base_addr / 4096;
                uint64_t end_page = (mmap->base_addr + mmap->length) / 4096;
                for (uint64_t i = start_page; i < end_page && i < PMM_MAX_PAGES; i++) {
                    pmm_bitmap[i / 8] &= (uint8_t)~(1U << (i % 8));
                }
            }
            mmap = (multiboot_mmap_entry_t*)((uintptr_t)mmap + mmap->size + 4);
        }
    }
    for (uint32_t i = 0; i < 0x100; i++) {
        BIT_SET(pmm_bitmap, i);
    }
    uint32_t kernel_start = 0x100000 / 4096;
    uint32_t kernel_end = ((uintptr_t)&_kernel_end + 4095) / 4096;
    for (uint32_t i = kernel_start; i < kernel_end && i < PMM_MAX_PAGES; i++) {
        BIT_SET(pmm_bitmap, i);
    }
    if (mb->flags & (1U << 3)) {
        multiboot_module_t* mod = (multiboot_module_t*)(uintptr_t)mb->mods_addr;
        for (uint32_t m = 0; m < mb->mods_count; m++) {
            uint32_t mod_start = mod[m].mod_start / 4096;
            uint32_t mod_end = (mod[m].mod_end + 4095) / 4096;
            for (uint32_t i = mod_start; i < mod_end && i < PMM_MAX_PAGES; i++) {
                BIT_SET(pmm_bitmap, i);
            }
        }
    }
}

void* pmm_alloc(void) {
    for (uint32_t i = 0; i < PMM_MAX_PAGES; i++) {
        if (!BIT_TEST(pmm_bitmap, i)) {
            BIT_SET(pmm_bitmap, i);
            return (void*)(uintptr_t)(i * 4096);
        }
    }
    panic("Out of Memory");
    return NULL;
}

void pmm_free(void* addr) {
    uint32_t page = (uint32_t)((uintptr_t)addr / 4096);
    if (page < PMM_MAX_PAGES) {
        pmm_bitmap[page / 8] &= (uint8_t)~(1U << (page % 8));
    }
}

void* pmm_alloc_pages(int count) {
    int start = -1;
    int found = 0;
    for (int i = 0; i < PMM_MAX_PAGES; i++) {
        if (!BIT_TEST(pmm_bitmap, i)) {
            if (start == -1) {
                start = i;
            }
            found++;
            if (found == count) {
                for (int j = start; j < start + count; j++) {
                    BIT_SET(pmm_bitmap, j);
                }
                return (void*)(uintptr_t)(start * 4096);
            }
        } else {
            start = -1;
            found = 0;
        }
    }
    panic("Out of Memory");
    return NULL;
}
