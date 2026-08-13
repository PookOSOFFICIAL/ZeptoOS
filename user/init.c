struct linux_dirent {
    unsigned long  d_ino;
    unsigned long  d_off;
    unsigned short d_reclen;
    char           d_name[];
};

int sys_write(int fd, const char *buf, int len) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a" (ret)
        : "a" (4), "b" (fd), "c" (buf), "d" (len)
        : "memory"
    );
    return ret;
}

int sys_read(int fd, char *buf, int len) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a" (ret)
        : "a" (3), "b" (fd), "c" (buf), "d" (len)
        : "memory"
    );
    return ret;
}

int sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count) {
    int ret;
    __asm__ volatile(
        "int $0x80"
        : "=a" (ret)
        : "a" (141), "b" (fd), "c" (dirp), "d" (count)
        : "memory"
    );
    return ret;
}

int string_compare(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void _start() {
    char buf[128];
    sys_write(1, "ZeptoOS Shell v1.0\n", 19);

    while (1) {
        sys_write(1, "zepto> ", 7);
        int len = sys_read(0, buf, 127);
        if (len <= 0) continue;

        buf[len - 1] = '\0';
        if (len > 1 && buf[len - 2] == '\r') {
            buf[len - 2] = '\0';
        }

        if (string_compare(buf, "help") == 0) {
            sys_write(1, "Available commands: help, ls, hello, clear, exit\n", 49);
        } else if (string_compare(buf, "ls") == 0) {
            char dbuf[1024];
            int nread = sys_getdents(0, (struct linux_dirent*)dbuf, 1024);
            int bpos = 0;
            while (bpos < nread) {
                struct linux_dirent *d = (struct linux_dirent*)(dbuf + bpos);
                sys_write(1, d->d_name, 0);
                int k = 0;
                while (d->d_name[k] != '\0') k++;
                sys_write(1, "\n", 1);
                bpos += d->d_reclen;
            }
        } else if (string_compare(buf, "hello") == 0) {
            sys_write(1, "Hello World With File System\n", 29);
        } else if (string_compare(buf, "clear") == 0) {
            sys_write(1, "\033[2J\033[H", 7);
        } else if (string_compare(buf, "exit") == 0) {
            sys_write(1, "Goodbye!\n", 9);
            break;
        } else if (buf[0] != '\0') {
            sys_write(1, "Unknown command: ", 17);
            sys_write(1, buf, 0);
            sys_write(1, "\n", 1);
        }
    }

    while (1);
}
