#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

extern void irq0_stub(void);
extern void irq1_stub(void);
extern void syscall_stub(void);

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)(uintptr_t)&idt;
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    idt_set_gate(32, (uint32_t)(uintptr_t)irq0_stub, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)(uintptr_t)irq1_stub, 0x08, 0x8E);
    idt_set_gate(0x80, (uint32_t)(uintptr_t)syscall_stub, 0x08, 0xEE);
    idt_load();
}
