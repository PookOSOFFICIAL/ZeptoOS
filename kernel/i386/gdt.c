#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
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

struct gdt_entry gdt[6];
struct gdt_ptr gp;
struct tss_entry tss;

extern void gdt_flush(uint32_t);
extern void tss_flush(void);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access       = access;
}

void write_tss(int num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof(struct tss_entry);

    gdt_set_gate(num, base, limit, 0x89, 0x40);

    for (int i = 0; i < sizeof(struct tss_entry); i++) {
        ((char*)&tss)[i] = 0;
    }

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x08;
    tss.ss = 0x10;
    tss.ds = 0x10;
    tss.es = 0x10;
    tss.fs = 0x10;
    tss.gs = 0x10;
    tss.iomap_base = sizeof(struct tss_entry);
}

void set_kernel_stack(uint32_t stack)
{
    tss.esp0 = stack;
}

void init_gdt()
{
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gp.base  = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    write_tss(5, 0x10, 0);

    gdt_flush((uint32_t)&gp);
    tss_flush();
}
