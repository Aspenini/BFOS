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

.PHONY: all clean run sysfs

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

run: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

clean:
	@echo "Cleaning build artifacts..."
	rm -f *.o *.bin *.elf *.iso *.img
	rm -f *.obj *.a *.so *.dylib
	rm -f *.swp *.swo *~ .DS_Store
	rm -f *.tmp *.bak *.log
	rm -f *.qcow2 *.vmdk
	rm -f sysfs_data.c sysfs_data.o
	rm -rf arch/*/boot.o arch/*/arch.o
	rm -rf .vscode .idea
	@echo "Clean complete."
