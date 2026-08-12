#include "scheduler.h"
#include "drv/vga.h"

task_t tasks[10];
int current_task = -1;
int task_count = 0;

void scheduler_init() {
    for (int i = 0; i < 10; i++) {
        tasks[i].active = 0;
    }
}

void task_create(void (*entry_point)()) {
    if (task_count >= 10) return;

    task_t *t = &tasks[task_count];
    t->pid = task_count + 1;
    t->active = 1;

    uint32_t *stack = (uint32_t *)(t->stack + 4096);

    *--stack = 0x202; 
    *--stack = 0x08;  
    *--stack = (uint32_t)entry_point;

    *--stack = 0; 
    *--stack = 0; 
    *--stack = 0; 
    *--stack = 0; 
    *--stack = 0; 
    *--stack = 0; 
    *--stack = 0; 
    *--stack = 0; 

    t->esp = (uint32_t)stack;
    task_count++;
}

uint32_t schedule(uint32_t esp) {
    if (task_count == 0) return esp;

    if (current_task != -1) {
        tasks[current_task].esp = esp;
    }

    current_task = (current_task + 1) % task_count;
    return tasks[current_task].esp;
}

uint32_t get_current_pid() {
    if (current_task == -1) return 0;
    return tasks[current_task].pid;
}

void print_pid(uint32_t pid) {
    char buf[16];
    int i = 0;
    if (pid == 0) {
        buf[i++] = '0';
    } else {
        while (pid > 0) {
            buf[i++] = (pid % 10) + '0';
            pid /= 10;
        }
    }
    buf[i] = '\0';
    
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }
    
    kprintf((unsigned char*)buf);
    kprintf((unsigned char*)" ");
}
