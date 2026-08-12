DIST = build/
CSRC = $(shell find kernel -name "*.c")
COBJ = $(CSRC:.c=.o)
ASSRC = $(shell find kernel -name "*.s")
ASOBJ = $(patsubst %.s,%.o,$(ASSRC))
SRC = $(CSRC) $(ASSRC)
OBJ = $(COBJ) $(ASOBJ)
CC = clang -target i386-elf
ASM = nasm
LD = ld.lld
CFLAGS = -ffreestanding -m32 -O2 -nostdlib -fno-builtin -mno-sse -mno-sse2 -msoft-float -Ikernel
ASMFLAGS = -f elf32
LDFLAGS = -T kernel/i386/linker.ld -m elf_i386
all: kernel.bin
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.s
	nasm $(ASMFLAGS) $< -o $@
kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^
iso: kernel.bin
	@mkdir -p iso/boot/grub
	@cp kernel.bin iso/boot/
	@cp grub.cfg iso/boot/grub/ 2>/dev/null || true
	@grub-mkrescue -o kernel.iso iso/ 2>/dev/null || grub2-mkrescue -o kernel.iso iso/
clean:
	rm -f $(OBJ) kernel.bin
	rm -rf iso/
	rm -f kernel.iso
.PHONY: all iso clean
