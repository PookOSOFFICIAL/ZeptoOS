#include "../lib/types.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[7];
static struct gdt_ptr gdt_pointer;
static struct tss_entry tss;

extern void gdt_flush(struct gdt_ptr* pointer);
extern void tss_flush(void);

static void gdt_set_gate(int index, uint8_t access, uint8_t granularity) {
    gdt[index].limit_low = 0xFFFF;
    gdt[index].base_low = 0;
    gdt[index].base_middle = 0;
    gdt[index].access = access;
    gdt[index].granularity = granularity;
    gdt[index].base_high = 0;
}

static void write_tss(void) {
    uint64_t base = (uint64_t)(uintptr_t)&tss;
    uint32_t limit = sizeof(tss) - 1;
    for (uint32_t i = 0; i < sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }
    tss.iomap_base = sizeof(tss);
    gdt[5].limit_low = limit & 0xFFFF;
    gdt[5].base_low = base & 0xFFFF;
    gdt[5].base_middle = (base >> 16) & 0xFF;
    gdt[5].access = 0x89;
    gdt[5].granularity = (limit >> 16) & 0x0F;
    gdt[5].base_high = (base >> 24) & 0xFF;
    gdt[6].limit_low = (base >> 32) & 0xFFFF;
    gdt[6].base_low = (base >> 48) & 0xFFFF;
    gdt[6].base_middle = 0;
    gdt[6].access = 0;
    gdt[6].granularity = 0;
    gdt[6].base_high = 0;
}

void set_kernel_stack(uintptr_t stack) {
    tss.rsp0 = stack;
}

void init_gdt(void) {
    gdt_set_gate(0, 0, 0);
    gdt_set_gate(1, 0x9A, 0xAF);
    gdt_set_gate(2, 0x92, 0xCF);
    gdt_set_gate(3, 0xFA, 0xAF);
    gdt_set_gate(4, 0xF2, 0xCF);
    write_tss();
    gdt_pointer.limit = sizeof(gdt) - 1;
    gdt_pointer.base = (uint64_t)(uintptr_t)&gdt;
    gdt_flush(&gdt_pointer);
    tss_flush();
}
