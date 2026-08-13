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
    uint32_t base;
} __attribute__((packed));

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[6];
static struct gdt_ptr gdt_pointer;
static struct tss_entry tss;

extern void gdt_flush(uint32_t pointer);
extern void tss_flush(void);

static void gdt_set_gate(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[index].base_low = base & 0xFFFF;
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    gdt[index].limit_low = limit & 0xFFFF;
    gdt[index].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
    gdt[index].access = access;
}

static void write_tss(int index, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)(uintptr_t)&tss;
    uint32_t limit = sizeof(tss) - 1;
    gdt_set_gate(index, base, limit, 0x89, 0x40);
    for (uint32_t i = 0; i < sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x08;
    tss.ss = 0x10;
    tss.ds = 0x10;
    tss.es = 0x10;
    tss.fs = 0x10;
    tss.gs = 0x10;
    tss.iomap_base = sizeof(tss);
}

void set_kernel_stack(uintptr_t stack) {
    tss.esp0 = (uint32_t)stack;
}

void init_gdt(void) {
    gdt_pointer.limit = sizeof(gdt) - 1;
    gdt_pointer.base = (uint32_t)(uintptr_t)&gdt;
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    write_tss(5, 0x10, 0);
    gdt_flush((uint32_t)(uintptr_t)&gdt_pointer);
    tss_flush();
}
