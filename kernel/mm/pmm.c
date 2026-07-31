#include "../lib/types.h"
#define PMM_MAX_PAGES 32768 // 128 mb
uint8_t pmm_bitmap[PMM_MAX_PAGES / 8];

#define BIT_SET(a, i) ((a)[(i) / 8] |= (1 << ((i) % 8)))
#define BIT_CLEAR(a, i) ((a)[(i) / 8] &= ~(1 << ((i) % 8)))
#define BIT_TEST(a, i) ((a)[(i) / 8] & (1 << ((i) % 8)))

void pmm_init() {
    for (int i = 0; i < 768; i++) {
	BIT_SET(pmm_bitmap, i);
    }
}

void* pmm_alloc() {
    for (int i = 0; i < PMM_MAX_PAGES; i++) {
        if (!(pmm_bitmap[i / 8] & (1 << (i % 8)))) {
            pmm_bitmap[i / 8] |= (1 << (i % 8));
            return (void*)(i * 4096 + 0x100000);
        }
    }
    return NULL;
}

void pmm_free(void* addr) {
    int page = ((uint32_t)addr - 0x100000) / 4096;
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
                return (void*)(start * 4096 + 0x100000);
            }
        } else {
            start = -1;
            found = 0;
        }
    }
    return NULL;
}
