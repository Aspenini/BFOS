# BFOS Makefile
# Default: x86_64
# Usage: make (x86_64) or make ARCH=arm32 (other architectures)

# Default architecture
ARCH ?= x86_64

# Architecture-specific toolchain configuration
ifeq ($(ARCH),x86_64)
    ASM = nasm
    CC = x86_64-elf-gcc
    LD = x86_64-elf-ld
    ASMFLAGS = -f elf32
    CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -DARCH_X86_64
    LDFLAGS = -m elf_i386 -T arch/x86_64/linker.ld
    BOOT_SRC = arch/x86_64/boot.asm
    BOOT_OBJ = arch/x86_64/boot.o
    ARCH_SRC = arch/x86_64/arch.c
    ARCH_OBJ = arch/x86_64/arch.o
    QEMU = qemu-system-i386
    QEMU_FLAGS = -kernel
    KERNEL_BIN = kernel.bin
    ISO_DIR = iso
    ISO_FILE = kernel.iso
else ifeq ($(ARCH),x86_32)
    ASM = nasm
    CC = i686-elf-gcc
    LD = i686-elf-ld
    ASMFLAGS = -f elf32
    CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -DARCH_X86_32
    LDFLAGS = -m elf_i386 -T arch/x86_32/linker.ld
    BOOT_SRC = arch/x86_32/boot.asm
    BOOT_OBJ = arch/x86_32/boot.o
    ARCH_SRC = arch/x86_32/arch.c
    ARCH_OBJ = arch/x86_32/arch.o
    QEMU = qemu-system-i386
    QEMU_FLAGS = -kernel
    KERNEL_BIN = kernel-$(ARCH).bin
    ISO_DIR = iso-$(ARCH)
    ISO_FILE = kernel-$(ARCH).iso
else ifeq ($(ARCH),arm32)
    ASM = arm-none-eabi-as
    CC = arm-none-eabi-gcc
    LD = arm-none-eabi-ld
    ASMFLAGS = -mcpu=cortex-a7 -mthumb
    CFLAGS = -mcpu=cortex-a7 -mthumb -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -DARCH_ARM32
    LDFLAGS = -T arch/arm32/linker.ld
    BOOT_SRC = arch/arm32/boot.S
    BOOT_OBJ = arch/arm32/boot.o
    ARCH_SRC = arch/arm32/arch.c
    ARCH_OBJ = arch/arm32/arch.o
    QEMU = qemu-system-arm
    QEMU_FLAGS = -M versatilepb -cpu arm1176 -kernel
    KERNEL_BIN = kernel-$(ARCH).bin
    ISO_DIR = iso-$(ARCH)
    ISO_FILE = kernel-$(ARCH).iso
else ifeq ($(ARCH),arm64)
    ASM = aarch64-none-elf-as
    CC = aarch64-none-elf-gcc
    LD = aarch64-none-elf-ld
    ASMFLAGS =
    CFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -DARCH_ARM64
    LDFLAGS = -T arch/arm64/linker.ld
    BOOT_SRC = arch/arm64/boot.S
    BOOT_OBJ = arch/arm64/boot.o
    ARCH_SRC = arch/arm64/arch.c
    ARCH_OBJ = arch/arm64/arch.o
    QEMU = qemu-system-aarch64
    QEMU_FLAGS = -M virt -cpu cortex-a57 -kernel
    KERNEL_BIN = kernel-$(ARCH).bin
    ISO_DIR = iso-$(ARCH)
    ISO_FILE = kernel-$(ARCH).iso
else ifeq ($(ARCH),riscv)
    ASM = riscv64-unknown-elf-as
    CC = riscv64-unknown-elf-gcc
    LD = riscv64-unknown-elf-ld
    ASMFLAGS =
    CFLAGS = -march=rv64imafdc -mabi=lp64d -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -DARCH_RISCV64
    LDFLAGS = -T arch/riscv/linker.ld
    BOOT_SRC = arch/riscv/boot.S
    BOOT_OBJ = arch/riscv/boot.o
    ARCH_SRC = arch/riscv/arch.c
    ARCH_OBJ = arch/riscv/arch.o
    QEMU = qemu-system-riscv64
    QEMU_FLAGS = -M virt -cpu rv64 -kernel
    KERNEL_BIN = kernel-$(ARCH).bin
    ISO_DIR = iso-$(ARCH)
    ISO_FILE = kernel-$(ARCH).iso
else
    $(error Unsupported architecture: $(ARCH). Supported: x86_64, x86_32, arm32, arm64, riscv)
endif

KERNEL_OBJ = $(BOOT_OBJ) $(ARCH_OBJ) kernel.o terminal.o bf_interpreter.o keyboard.o filesystem.o shell.o sysfs_data.o config.o framebuffer.o uart.o

.PHONY: all clean run run-iso iso sysfs

all: sysfs $(KERNEL_BIN)

sysfs:
	@python3 build_sysfs.py

sysfs_data.o: sysfs_data.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

sysfs_data.c: build_sysfs.py
	@python3 build_sysfs.py

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel built for $(ARCH): $(KERNEL_BIN)"

$(BOOT_OBJ): $(BOOT_SRC)
	@mkdir -p $(dir $(BOOT_OBJ))
	$(ASM) $(ASMFLAGS) -o $@ $<

$(ARCH_OBJ): $(ARCH_SRC) arch.h kernel.h
	@mkdir -p $(dir $(ARCH_OBJ))
	$(CC) $(CFLAGS) -c -o $@ $<

kernel.o: kernel.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

terminal.o: terminal.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

bf_interpreter.o: bf_interpreter.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

keyboard.o: keyboard.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

filesystem.o: filesystem.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

shell.o: shell.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

config.o: config.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

framebuffer.o: framebuffer.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

uart.o: uart.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

iso: $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	@echo "menuentry \"BFOS ($(ARCH))\" {" > $(ISO_DIR)/boot/grub/grub.cfg
	@echo "  multiboot /boot/$(KERNEL_BIN)" >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo "  boot" >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo "}" >> $(ISO_DIR)/boot/grub/grub.cfg
	@GRUB_MKRESCUE=""; \
	if command -v i686-elf-grub-mkrescue >/dev/null 2>&1; then \
		GRUB_MKRESCUE="i686-elf-grub-mkrescue"; \
	elif command -v x86_64-elf-grub-mkrescue >/dev/null 2>&1; then \
		GRUB_MKRESCUE="x86_64-elf-grub-mkrescue"; \
	elif command -v grub-mkrescue >/dev/null 2>&1; then \
		GRUB_MKRESCUE="grub-mkrescue"; \
	fi; \
	if [ -z "$$GRUB_MKRESCUE" ]; then \
		echo "Error: grub-mkrescue not found. Install with: brew install i686-elf-grub"; \
		echo "Or use 'make run' to run the kernel directly with QEMU."; \
		exit 1; \
	else \
		$$GRUB_MKRESCUE -o $(ISO_FILE) $(ISO_DIR); \
		echo "ISO created: $(ISO_FILE)"; \
	fi

run: $(KERNEL_BIN)
	$(QEMU) $(QEMU_FLAGS) $(KERNEL_BIN)

run-iso: iso
	$(QEMU) -cdrom $(ISO_FILE)

clean:
	@echo "Cleaning build artifacts..."
	rm -f *.o *.bin *.elf *.iso *.img
	rm -f *.obj *.a *.so *.dylib
	rm -f *.swp *.swo *~ .DS_Store
	rm -f *.tmp *.bak *.log
	rm -f *.qcow2 *.vmdk
	rm -f sysfs_data.c sysfs_data.o
	rm -rf iso iso-*
	rm -rf arch/*/boot.o arch/*/arch.o
	rm -rf .vscode .idea
	@echo "Clean complete."
