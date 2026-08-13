ARCH ?= i386

ifeq ($(ARCH),i386)
TARGET := i386-elf
LD_EMULATION := elf_i386
ARCH_CFLAGS := -m32 -mno-sse -mno-sse2 -msoft-float
else ifeq ($(ARCH),x86_64)
TARGET := x86_64-elf
LD_EMULATION := elf_x86_64
ARCH_CFLAGS := -m64 -mno-red-zone -mcmodel=small -mno-sse -mno-sse2 -mno-mmx
else
$(error ARCH must be i386 or x86_64)
endif

BUILD_DIR := build/$(ARCH)
CC := clang
LD := ld.lld
CFLAGS := -target $(TARGET) -ffreestanding -O2 -nostdlib -fno-builtin -fno-stack-protector -fno-pic -fno-asynchronous-unwind-tables $(ARCH_CFLAGS)

all: $(BUILD_DIR)/initrd.tar

$(BUILD_DIR)/init.o: init.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/init: $(BUILD_DIR)/init.o
	$(LD) -m $(LD_EMULATION) -Ttext 0x800000 -e _start -o $@ $<

$(BUILD_DIR)/initrd.tar: $(BUILD_DIR)/init
	tar -C $(BUILD_DIR) -cf $@ init

clean:
	rm -rf build

.PHONY: all clean
