#include "syscall.h"
#include "../core/vga.h"
#include "../core/keyboard.h"
#include "../core/scheduler.h"
#include "../fs/vfs.h"
#include "../lib/types.h"

struct regs {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
};

struct linux_dirent {
    uint64_t d_ino;
    uint64_t d_off;
    uint16_t d_reclen;
    char d_name[];
};

static int64_t sys_write(int fd, const char* buf, uint64_t len) {
    if (fd == 1 || fd == 2) {
        for (uint64_t i = 0; i < len; i++) {
            char str[2] = {buf[i], '\0'};
            kprintf(str);
        }
        return (int64_t)len;
    }
    return -1;
}

static int64_t sys_read(int fd, char* buf, uint64_t len) {
    if (fd != 0) {
        return 0;
    }
    uint64_t written = 0;
    while (written < len) {
        char c = keyboard_get_char();
        if (c == '\b') {
            if (written > 0) {
                written--;
                kprintf("\b \b");
            }
            continue;
        }
        if (c == '\n' || c == '\r') {
            buf[written++] = '\n';
            kprintf("\n");
            return (int64_t)written;
        }
        buf[written++] = c;
        char echo[2] = {c, '\0'};
        kprintf(echo);
    }
    return (int64_t)written;
}

static int64_t sys_open(const char* path, int flags, int mode) {
    (void)flags;
    (void)mode;
    if (!vfs_resolve_path(path)) {
        return -1;
    }
    return 3;
}

static int64_t sys_close(int fd) {
    (void)fd;
    return 0;
}

static int64_t sys_getdents(unsigned int fd, struct linux_dirent* dirp, unsigned int count) {
    (void)fd;
    struct vfs_node* node = vfs_root();
    if (!node) {
        return -1;
    }
    uint32_t pos = 0;
    int index = 0;
    struct dirent* de;
    while ((de = vfs_readdir(node, index)) != NULL) {
        int name_len = 0;
        while (de->name[name_len] != '\0') {
            name_len++;
        }
        uint16_t reclen = (uint16_t)(sizeof(struct linux_dirent) + name_len + 1);
        reclen = (uint16_t)((reclen + 7) & ~7);
        if (pos + reclen > count) {
            break;
        }
        struct linux_dirent* entry = (struct linux_dirent*)((uint8_t*)dirp + pos);
        entry->d_ino = de->ino;
        entry->d_off = pos + reclen;
        entry->d_reclen = reclen;
        for (int i = 0; i < name_len; i++) {
            entry->d_name[i] = de->name[i];
        }
        entry->d_name[name_len] = '\0';
        pos += reclen;
        index++;
    }
    return pos;
}

static int64_t sys_mount(const char* special, const char* dir, const char* fstype, uint64_t flags, const void* data) {
    (void)special;
    (void)fstype;
    (void)flags;
    return vfs_mount(dir, (struct vfs_node*)data);
}

static int64_t sys_umount(const char* special, int flags) {
    (void)flags;
    return vfs_umount(special);
}

static int64_t sys_mkdir(const char* pathname, int mode) {
    return vfs_mkdir(pathname, mode);
}

static int64_t sys_link(const char* oldpath, const char* newpath) {
    return vfs_link(oldpath, newpath);
}

static int64_t sys_unlink(const char* pathname) {
    return vfs_unlink(pathname);
}

static void sys_exit(int code) {
    (void)code;
    task_exit();
}

void syscall_handler(struct regs* regs) {
    switch (regs->rax) {
        case 1:
            sys_exit((int)regs->rdi);
            break;
        case 3:
            regs->rax = sys_read((int)regs->rdi, (char*)(uintptr_t)regs->rsi, regs->rdx);
            break;
        case 4:
            regs->rax = sys_write((int)regs->rdi, (const char*)(uintptr_t)regs->rsi, regs->rdx);
            break;
        case 5:
            regs->rax = sys_open((const char*)(uintptr_t)regs->rdi, (int)regs->rsi, (int)regs->rdx);
            break;
        case 6:
            regs->rax = sys_close((int)regs->rdi);
            break;
        case 9:
            regs->rax = sys_link((const char*)(uintptr_t)regs->rdi, (const char*)(uintptr_t)regs->rsi);
            break;
        case 10:
            regs->rax = sys_unlink((const char*)(uintptr_t)regs->rdi);
            break;
        case 21:
            regs->rax = sys_mount((const char*)(uintptr_t)regs->rdi, (const char*)(uintptr_t)regs->rsi, (const char*)(uintptr_t)regs->rdx, 0, NULL);
            break;
        case 22:
            regs->rax = sys_umount((const char*)(uintptr_t)regs->rdi, (int)regs->rsi);
            break;
        case 39:
            regs->rax = sys_mkdir((const char*)(uintptr_t)regs->rdi, (int)regs->rsi);
            break;
        case 141:
            regs->rax = sys_getdents((unsigned int)regs->rdi, (struct linux_dirent*)(uintptr_t)regs->rsi, (unsigned int)regs->rdx);
            break;
        default:
            regs->rax = (uint64_t)-1;
            break;
    }
}
