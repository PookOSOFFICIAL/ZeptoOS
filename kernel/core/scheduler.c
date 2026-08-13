#include "scheduler.h"
#include "vga.h"

#define MAX_TASKS 10

static task_t tasks[MAX_TASKS];
static int task_count;
static int current_task;

void set_kernel_stack(uintptr_t stack);

void scheduler_init(void) {
    task_count = 0;
    current_task = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].active = 0;
    }
}

void task_create_user(void (*entry_point)(void)) {
    if (task_count >= MAX_TASKS) {
        return;
    }

    task_t* task = &tasks[task_count];
    task->pid = (uint32_t)(task_count + 1);
    task->active = 1;
#ifdef __x86_64__
    uint64_t* kstack = (uint64_t*)(task->kernel_stack + sizeof(task->kernel_stack));
    uint64_t* ustack = (uint64_t*)(task->user_stack + sizeof(task->user_stack));
    *--kstack = 0x23;
    *--kstack = (uint64_t)(uintptr_t)ustack;
    *--kstack = 0x202;
    *--kstack = 0x1B;
    *--kstack = (uint64_t)(uintptr_t)entry_point;
    for (int i = 0; i < 15; i++) {
        *--kstack = 0;
    }
    task->stack_pointer = (uintptr_t)kstack;
#else
    uint32_t* kstack = (uint32_t*)(task->kernel_stack + sizeof(task->kernel_stack));
    uint32_t* ustack = (uint32_t*)(task->user_stack + sizeof(task->user_stack));
    *--kstack = 0x23;
    *--kstack = (uint32_t)(uintptr_t)ustack;
    *--kstack = 0x202;
    *--kstack = 0x1B;
    *--kstack = (uint32_t)(uintptr_t)entry_point;
    for (int i = 0; i < 8; i++) {
        *--kstack = 0;
    }
    task->stack_pointer = (uintptr_t)kstack;
#endif
    task_count++;
}

void task_exit(void) {
    if (current_task != -1) {
        tasks[current_task].active = 0;
    }
    asm volatile("int $32");
}

uintptr_t schedule(uintptr_t stack_pointer) {
    if (task_count == 0) {
        return stack_pointer;
    }
    if (current_task != -1) {
        tasks[current_task].stack_pointer = stack_pointer;
    }

    int next_task = current_task;
    for (int i = 0; i < task_count; i++) {
        next_task = (next_task + 1) % task_count;
        if (tasks[next_task].active) {
            current_task = next_task;
            set_kernel_stack((uintptr_t)(tasks[current_task].kernel_stack + sizeof(tasks[current_task].kernel_stack)));
            return tasks[current_task].stack_pointer;
        }
    }
    return stack_pointer;
}

uint32_t get_current_pid(void) {
    if (current_task == -1) {
        return 0;
    }
    return tasks[current_task].pid;
}

void print_pid(uint32_t pid) {
    char buf[16];
    int i = 0;
    if (pid == 0) {
        buf[i++] = '0';
    } else {
        while (pid > 0) {
            buf[i++] = (char)((pid % 10) + '0');
            pid /= 10;
        }
    }
    buf[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }
    kprintf(buf);
    kprintf(" ");
}
