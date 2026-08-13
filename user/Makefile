CC = clang -target i386-elf
CFLAGS = -ffreestanding -m32 -O2 -nostdlib -fno-builtin

all: initrd.tar

init: init.c
	$(CC) $(CFLAGS) -c -o init.o init.c
	ld.lld -Ttext 0x800000 -o init init.o

initrd.tar: init
	tar -cvf initrd.tar init

clean:
	rm -f init initrd.tar

.PHONY: all clean
