ASM = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

ASMFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL_OBJ = boot.o kernel.o terminal.o bf_interpreter.o keyboard.o filesystem.o shell.o sysfs_data.o config.o
KERNEL_BIN = kernel.bin
ISO_DIR = iso
ISO_FILE = kernel.iso

.PHONY: all clean run iso sysfs

all: sysfs $(KERNEL_BIN)

sysfs:
	@python3 build_sysfs.py

sysfs_data.o: sysfs_data.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

sysfs_data.c: build_sysfs.py
	@python3 build_sysfs.py

$(KERNEL_BIN): $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel built: $(KERNEL_BIN)"

boot.o: boot.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

terminal.o: terminal.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

bf_interpreter.o: bf_interpreter.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

keyboard.o: keyboard.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

filesystem.o: filesystem.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

shell.o: shell.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

config.o: config.c kernel.h
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

clean:
	@echo "Cleaning build artifacts..."
	rm -f *.o *.bin *.elf *.iso *.img
	rm -f *.obj *.a *.so *.dylib
	rm -f *.swp *.swo *~ .DS_Store
	rm -f *.tmp *.bak *.log
	rm -f *.qcow2 *.vmdk
	rm -f sysfs_data.c sysfs_data.o
	rm -rf $(ISO_DIR)
	rm -rf .vscode .idea
	@echo "Clean complete."

