#include "io.h"
#define PIT_COMMAND  0x43
#define PIT_CHANNEL0 0x40
#define PIT_CHANNEL1 0x41
#define PIT_CHANNEL2 0x42
#define PIT 1193182

void pit_set_frequency(uint32_t frequency) {
    uint16_t divisor = PIT / frequency;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}
void pit_init() {
    pit_set_frequency(100);
}
