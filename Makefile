ASM = nasm
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

ASMFLAGS = -f elf32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL_OBJ = boot.o kernel.o terminal.o bf_interpreter.o keyboard.o filesystem.o shell.o
KERNEL_BIN = kernel.bin
ISO_DIR = iso
ISO_FILE = kernel.iso

.PHONY: all clean run iso

all: $(KERNEL_BIN)

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

iso: $(KERNEL_BIN)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	@echo "menuentry \"BFOS\" {" > $(ISO_DIR)/boot/grub/grub.cfg
	@echo "  multiboot /boot/$(KERNEL_BIN)" >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo "  boot" >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo "}" >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_FILE) $(ISO_DIR)
	@echo "ISO created: $(ISO_FILE)"

run: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

run-iso: iso
	qemu-system-i386 -cdrom $(ISO_FILE)

clean:
	rm -f $(KERNEL_OBJ) $(KERNEL_BIN) $(ISO_FILE)
	rm -rf $(ISO_DIR)

