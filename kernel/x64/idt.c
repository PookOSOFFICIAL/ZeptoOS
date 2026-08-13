#include "idt.h"

static struct idt_entry idt[256];
static struct idt_ptr idt_pointer;

extern void irq0_stub(void);
extern void irq1_stub(void);
extern void syscall_stub(void);

static void idt_set_gate(uint8_t index, uint64_t base, uint16_t selector, uint8_t flags) {
    idt[index].offset_low = base & 0xFFFF;
    idt[index].selector = selector;
    idt[index].ist = 0;
    idt[index].flags = flags;
    idt[index].offset_middle = (base >> 16) & 0xFFFF;
    idt[index].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;
}

void idt_install(void) {
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    idt_set_gate(32, (uint64_t)(uintptr_t)irq0_stub, 0x08, 0x8E);
    idt_set_gate(33, (uint64_t)(uintptr_t)irq1_stub, 0x08, 0x8E);
    idt_set_gate(0x80, (uint64_t)(uintptr_t)syscall_stub, 0x08, 0xEE);
    idt_pointer.limit = sizeof(idt) - 1;
    idt_pointer.base = (uint64_t)(uintptr_t)&idt;
    idt_load(&idt_pointer);
}
