# BFOS Makefile
# Simple build system for x86_64

ASM = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

ASMFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra -DARCH_X86_64
LDFLAGS = -m elf_i386 -T arch/x86_64/linker.ld

KERNEL_OBJ = arch/x86_64/boot.o arch/x86_64/arch.o kernel.o terminal.o bf_interpreter.o keyboard.o filesystem.o shell.o sysfs_data.o config.o framebuffer.o uart.o
KERNEL_BIN = kernel.bin
ISO_DIR = iso
ISO_FILE = kernel.iso
FLOPPY_IMG = kernel.img

.PHONY: all clean run run-iso run-floppy iso floppy sysfs

all: sysfs $(KERNEL_BIN)

sysfs:
	@python3 build_sysfs.py

sysfs_data.o: sysfs_data.c kernel.h arch.h
	$(CC) $(CFLAGS) -c -o $@ $<

sysfs_data.c: build_sysfs.py
	@python3 build_sysfs.py

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel built: $(KERNEL_BIN)"

arch/x86_64/boot.o: arch/x86_64/boot.asm
	@mkdir -p arch/x86_64
	$(ASM) $(ASMFLAGS) -o $@ $<

arch/x86_64/arch.o: arch/x86_64/arch.c arch.h kernel.h
	@mkdir -p arch/x86_64
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
	@echo "menuentry \"BFOS\" {" > $(ISO_DIR)/boot/grub/grub.cfg
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
	qemu-system-i386 -kernel $(KERNEL_BIN)

run-iso: iso
	qemu-system-i386 -cdrom $(ISO_FILE)

floppy: $(KERNEL_BIN)
	@echo "Creating 1.44MB floppy disk image..."
	@rm -f $(FLOPPY_IMG)
	@dd if=/dev/zero of=$(FLOPPY_IMG) bs=1024 count=1440 2>/dev/null || \
		dd if=/dev/zero of=$(FLOPPY_IMG) bs=512 count=2880 2>/dev/null
	@echo "Setting up GRUB on floppy..."
	@mkdir -p floppy-tmp/boot/grub
	@cp $(KERNEL_BIN) floppy-tmp/boot/
	@echo "menuentry \"BFOS\" {" > floppy-tmp/boot/grub/grub.cfg
	@echo "  multiboot /boot/$(KERNEL_BIN)" >> floppy-tmp/boot/grub/grub.cfg
	@echo "  boot" >> floppy-tmp/boot/grub/grub.cfg
	@echo "}" >> floppy-tmp/boot/grub/grub.cfg
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
		echo "Floppy image created but not bootable. You'll need to install GRUB manually."; \
		rm -rf floppy-tmp; \
		exit 1; \
	else \
		$$GRUB_MKRESCUE --format=i386-pc -o $(FLOPPY_IMG) floppy-tmp 2>/dev/null || \
		$$GRUB_MKRESCUE -d /opt/homebrew/Cellar/i686-elf-grub/*/lib/i686-elf/grub/i386-pc -o $(FLOPPY_IMG) floppy-tmp 2>/dev/null || \
		$$GRUB_MKRESCUE -d /usr/lib/grub/i386-pc -o $(FLOPPY_IMG) floppy-tmp 2>/dev/null || \
		$$GRUB_MKRESCUE -o $(FLOPPY_IMG) floppy-tmp; \
		if [ $$? -eq 0 ]; then \
			IMG_SIZE=$$(stat -f%z $(FLOPPY_IMG) 2>/dev/null || stat -c%s $(FLOPPY_IMG) 2>/dev/null || echo "0"); \
			if [ $$IMG_SIZE -gt 1474560 ]; then \
				echo "Warning: Image size ($$IMG_SIZE bytes) exceeds 1.44MB floppy capacity (1474560 bytes)"; \
				echo "Truncating to 1.44MB..."; \
				dd if=$(FLOPPY_IMG) of=$(FLOPPY_IMG).tmp bs=1474560 count=1 2>/dev/null; \
				mv $(FLOPPY_IMG).tmp $(FLOPPY_IMG); \
			fi; \
			echo "Floppy image created: $(FLOPPY_IMG) ($$(stat -f%z $(FLOPPY_IMG) 2>/dev/null || stat -c%s $(FLOPPY_IMG) 2>/dev/null) bytes)"; \
		else \
			echo "Warning: grub-mkrescue failed. Creating raw image - you may need to install GRUB manually."; \
		fi; \
		rm -rf floppy-tmp; \
	fi

run-floppy: floppy
	qemu-system-i386 -fda $(FLOPPY_IMG)

clean:
	@echo "Cleaning build artifacts..."
	rm -f *.o *.bin *.elf *.iso *.img
	rm -f *.obj *.a *.so *.dylib
	rm -f *.swp *.swo *~ .DS_Store
	rm -f *.tmp *.bak *.log
	rm -f *.qcow2 *.vmdk
	rm -f sysfs_data.c sysfs_data.o
	rm -rf iso
	rm -rf floppy-tmp
	rm -rf arch/*/boot.o arch/*/arch.o
	rm -rf .vscode .idea
	@echo "Clean complete."
