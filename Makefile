ARCH ?= i386

ifeq ($(ARCH),i386)
ARCH_DIR := i386
TARGET := i386-elf
NASM_FORMAT := elf32
LD_EMULATION := elf_i386
QEMU := qemu-system-i386
ARCH_CFLAGS := -m32 -mno-sse -mno-sse2 -msoft-float
else ifeq ($(ARCH),x86_64)
ARCH_DIR := x64
TARGET := x86_64-elf
NASM_FORMAT := elf64
LD_EMULATION := elf_x86_64
QEMU := qemu-system-x86_64
ARCH_CFLAGS := -m64 -mno-red-zone -mcmodel=small -mno-sse -mno-sse2 -mno-mmx
else
$(error ARCH must be i386 or x86_64)
endif

BUILD_DIR := build/$(ARCH)
OBJ_DIR := $(BUILD_DIR)/obj
KERNEL := $(BUILD_DIR)/kernel.bin
ISO := $(BUILD_DIR)/kernel.iso
ISO_DIR := $(BUILD_DIR)/iso

CC := clang
LD := ld.lld
CFLAGS := -target $(TARGET) -ffreestanding -O2 -nostdlib -fno-builtin -fno-stack-protector -fno-pic -fno-asynchronous-unwind-tables -Wall -Wextra -Wno-unused-parameter -Ikernel $(ARCH_CFLAGS)
NASMFLAGS := -f $(NASM_FORMAT)
LDFLAGS := -m $(LD_EMULATION) -T kernel/$(ARCH_DIR)/linker.ld

COMMON_C := $(sort $(wildcard kernel/core/*.c) $(wildcard kernel/fs/*.c) kernel/mm/pmm.c kernel/main.c)
ARCH_C := kernel/$(ARCH_DIR)/elf.c kernel/$(ARCH_DIR)/gdt.c kernel/$(ARCH_DIR)/idt.c kernel/$(ARCH_DIR)/syscall.c kernel/$(ARCH_DIR)/vmm.c
ARCH_S := kernel/$(ARCH_DIR)/boot.s kernel/$(ARCH_DIR)/gdt_flush.s kernel/$(ARCH_DIR)/irq.s kernel/$(ARCH_DIR)/kidt.s kernel/$(ARCH_DIR)/syscall_stub.s
C_SRC := $(COMMON_C) $(ARCH_C)
C_OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SRC))
S_OBJ := $(patsubst %.s,$(OBJ_DIR)/%.o,$(ARCH_S))
OBJ := $(C_OBJ) $(S_OBJ)

all: $(KERNEL)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s
	@mkdir -p $(@D)
	nasm $(NASMFLAGS) $< -o $@

$(KERNEL): $(OBJ)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -o $@ $^

iso: $(KERNEL)
	$(MAKE) -C user ARCH=$(ARCH)
	@rm -rf $(ISO_DIR)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	@cp user/build/$(ARCH)/initrd.tar $(ISO_DIR)/boot/initrd.tar
	@cp grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO) $(ISO_DIR)

run: iso
	$(QEMU) -m 128M -cdrom $(ISO) -serial stdio -display none -no-reboot -no-shutdown

clean:
	rm -rf build
	$(MAKE) -C user clean

.PHONY: all iso run clean
