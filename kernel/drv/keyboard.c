#include "keyboard.h"
#include "../i386/io.h"
#include "../i386/pit.h"
#include "../scheduler.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define POLL_INTERVAL 100000

static char kbdus[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',
    'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, '-', 0, 0, 0,
    '+', 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
};

static char keyboard_buffer[256];
static int head = 0;
static int tail = 0;
static volatile uint32_t poll_counter = 0;

void keyboard_init() {
}

void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    if (!(scancode & 0x80)) {
        char c = kbdus[scancode];
        if (c) {
            int next = (head + 1) % 256;
            if (next != tail) {
                keyboard_buffer[head] = c;
                head = next;
            }
        }
    }
}

void keyboard_poll() {
    poll_counter++;
    if (poll_counter >= POLL_INTERVAL) {
        poll_counter = 0;
        if (inb(KEYBOARD_STATUS_PORT) & 1) {
            keyboard_handler();
        }
    }
}

int keyboard_has_char() {
    keyboard_poll();
    return head != tail;
}

char keyboard_get_char() {
    while (!keyboard_has_char());
    char c = keyboard_buffer[tail];
    tail = (tail + 1) % 256;
    return c;
}
