#include "syscall.h"
#include "drv/vga.h"
#include "scheduler.h"

struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

int sys_write(int fd, const char *buf, uint32_t len) {
    if (fd == 1 || fd == 2) {
        for (uint32_t i = 0; i < len; i++) {
            char c = buf[i];
            char str[2] = {c, '\0'};
            kprintf((unsigned char*)str);
        }
        return len;
    }
    return -1;
}

void sys_exit(int code) {
    task_exit();
}

void syscall_handler(struct regs *r) {
    if (r->eax == 1) {
        sys_exit(r->ebx);
    } else if (r->eax == 4) {
        r->eax = sys_write(r->ebx, (const char*)r->ecx, r->edx);
    }
}
