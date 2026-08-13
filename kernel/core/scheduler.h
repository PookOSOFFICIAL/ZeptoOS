#pragma once

#include "../lib/types.h"

typedef struct {
    uintptr_t stack_pointer;
    uint32_t pid;
    uint8_t kernel_stack[4096] __attribute__((aligned(16)));
    uint8_t user_stack[4096] __attribute__((aligned(16)));
    uint32_t active;
} task_t;

void scheduler_init(void);
void task_create_user(void (*entry_point)(void));
void task_exit(void);
uintptr_t schedule(uintptr_t stack_pointer);
uint32_t get_current_pid(void);
void print_pid(uint32_t pid);
