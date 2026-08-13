struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

static int string_length(const char* str) {
    int length = 0;
    while (str[length]) {
        length++;
    }
    return length;
}

static long syscall3(long number, long arg1, long arg2, long arg3) {
#if defined(__x86_64__)
    __asm__ volatile(
        "int $0x80"
        : "+a"(number)
        : "D"(arg1), "S"(arg2), "d"(arg3)
        : "memory", "cc"
    );
#else
    __asm__ volatile(
        "int $0x80"
        : "+a"(number)
        : "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory", "cc"
    );
#endif
    return number;
}

static long sys_write(int fd, const char* buf, int len) {
    return syscall3(4, fd, (long)buf, len);
}

static long sys_read(int fd, char* buf, int len) {
    return syscall3(3, fd, (long)buf, len);
}

static long sys_getdents(unsigned int fd, struct linux_dirent* dirp, unsigned int count) {
    return syscall3(141, fd, (long)dirp, count);
}

static int string_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void write_string(const char* str) {
    sys_write(1, str, string_length(str));
}

void _start(void) {
    char buf[128];
    write_string("ZeptoOS Shell v1.0\n");
    for (;;) {
        write_string("zepto> ");
        int len = (int)sys_read(0, buf, 127);
        if (len <= 0) {
            continue;
        }
        buf[len - 1] = '\0';
        if (len > 1 && buf[len - 2] == '\r') {
            buf[len - 2] = '\0';
        }
        if (string_compare(buf, "help") == 0) {
            write_string("Available commands: help, ls, hello, clear, exit\n");
        } else if (string_compare(buf, "ls") == 0) {
            char dbuf[1024];
            int nread = (int)sys_getdents(0, (struct linux_dirent*)dbuf, sizeof(dbuf));
            int bpos = 0;
            while (bpos < nread) {
                struct linux_dirent* d = (struct linux_dirent*)(dbuf + bpos);
                write_string(d->d_name);
                write_string("\n");
                bpos += d->d_reclen;
            }
        } else if (string_compare(buf, "hello") == 0) {
            write_string("Hello World With File System\n");
        } else if (string_compare(buf, "clear") == 0) {
            write_string("\033[2J\033[H");
        } else if (string_compare(buf, "exit") == 0) {
            write_string("Goodbye!\n");
            break;
        } else if (buf[0] != '\0') {
            write_string("Unknown command: ");
            write_string(buf);
            write_string("\n");
        }
    }
    for (;;) {
    }
}
