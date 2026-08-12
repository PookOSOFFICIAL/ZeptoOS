#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "lib/types.h"

typedef struct {
    uint32_t esp;
    uint32_t pid;
    uint8_t stack[4096];
    uint32_t active;
} task_t;

void scheduler_init();
void task_create(void (*entry_point)());
uint32_t schedule(uint32_t esp);
uint32_t get_current_pid();
void print_pid(uint32_t pid);

#endif
