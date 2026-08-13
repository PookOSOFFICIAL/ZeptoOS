#include "syscall.h"
#include "../drv/vga.h"
#include "../drv/keyboard.h"
#include "../scheduler.h"
#include "../fs/vfs.h"

struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

struct linux_dirent {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
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

int sys_read(int fd, char *buf, uint32_t len) {
    if (fd == 0) {
        for (uint32_t i = 0; i < len; i++) {
            char c = keyboard_get_char();
            buf[i] = c;
            if (c == '\n' || c == '\r') {
                buf[i] = '\n';
                char echo[2] = {'\n', '\0'};
                kprintf((unsigned char*)echo);
                return i + 1;
            } else if (c == '\b') {
                if (i > 0) {
                    i -= 2;
                    kprintf((unsigned char*)"\b \b");
                } else {
                    i--;
                }
            } else {
                char echo[2] = {c, '\0'};
                kprintf((unsigned char*)echo);
            }
        }
        return len;
    }
    return 0;
}

int sys_open(const char *path, int flags, int mode) {
    struct vfs_node* node = vfs_resolve_path(path);
    if (!node) return -1;
    return 3;
}

int sys_close(int fd) {
    return 0;
}

int sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count) {
    struct vfs_node* node = vfs_root();
    if (!node) return -1;
    
    uint32_t pos = 0;
    int index = 0;
    struct dirent* de;
    
    while ((de = vfs_readdir(node, index)) != (struct dirent*)0) {
        int name_len = 0;
        while (de->name[name_len] != '\0') name_len++;
        
        uint16_t reclen = sizeof(struct linux_dirent) + name_len + 1;
        reclen = (reclen + 3) & ~3;
        
        if (pos + reclen > count) break;
        
        struct linux_dirent *ld = (struct linux_dirent*)((char*)dirp + pos);
        ld->d_ino = de->ino;
        ld->d_off = pos + reclen;
        ld->d_reclen = reclen;
        
        int i = 0;
        for (; i < name_len; i++) {
            ld->d_name[i] = de->name[i];
        }
        ld->d_name[i] = '\0';
        
        pos += reclen;
        index++;
    }
    return pos;
}

int sys_mount(const char *special, const char *dir, const char *fstype, unsigned long rwflag, const void *data) {
    return vfs_mount(dir, (struct vfs_node*)data);
}

int sys_umount(const char *special, int flags) {
    return vfs_umount(special);
}

int sys_mkdir(const char *pathname, int mode) {
    return vfs_mkdir(pathname, mode);
}

int sys_link(const char *oldpath, const char *newpath) {
    return vfs_link(oldpath, newpath);
}

int sys_unlink(const char *pathname) {
    return vfs_unlink(pathname);
}

void sys_exit(int code) {
    task_exit();
}

void syscall_handler(struct regs *r) {
    switch (r->eax) {
        case 1:
            sys_exit(r->ebx);
            break;
        case 3:
            r->eax = sys_read(r->ebx, (char*)r->ecx, r->edx);
            break;
        case 4:
            r->eax = sys_write(r->ebx, (const char*)r->ecx, r->edx);
            break;
        case 5:
            r->eax = sys_open((const char*)r->ebx, r->ecx, r->edx);
            break;
        case 6:
            r->eax = sys_close(r->ebx);
            break;
        case 9:
            r->eax = sys_link((const char*)r->ebx, (const char*)r->ecx);
            break;
        case 10:
            r->eax = sys_unlink((const char*)r->ebx);
            break;
        case 21:
            r->eax = sys_mount((const char*)r->ebx, (const char*)r->ecx, (const char*)r->edx, 0, NULL);
            break;
        case 22:
            r->eax = sys_umount((const char*)r->ebx, r->ecx);
            break;
        case 39:
            r->eax = sys_mkdir((const char*)r->ebx, r->ecx);
            break;
        case 141:
            r->eax = sys_getdents(r->ebx, (struct linux_dirent*)r->ecx, r->edx);
            break;
        default:
            r->eax = -1;
            break;
    }
}
