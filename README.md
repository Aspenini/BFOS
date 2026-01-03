# BFOS - Brainfuck Operating System

A runtime Brainfuck compilation operating system, similar to TempleOS/HolyC.

## Concept

- **Bootloader & Low-Level Kernel**: Written in C/Assembly
- **Everything Else**: Runs in Brainfuck, compiled at runtime
- **No Binaries**: Just Brainfuck source files
- **Ring 0**: All code runs in kernel mode
- **Runtime Compilation**: Brainfuck code is compiled to memory and executed on-the-fly

## Architecture

```
Bootloader (ASM) → Kernel (C) → Brainfuck Interpreter (C) → Brainfuck Programs
```

The kernel boots, initializes hardware, and provides a Brainfuck interpreter. All user code is written in Brainfuck and executed directly in memory.

## Building

### Prerequisites

- `nasm` - NASM assembler
- `gcc` - GCC compiler (32-bit support)
- `ld` - GNU linker
- `qemu-system-i386` (for running in VM)

### Build Steps

```bash
# Build the kernel
make

# Run in QEMU
make run

# Or create ISO and run
make iso
make run-iso
```

## Files

- `kernel.c` - Main kernel entry point
- `kernel.h` - Common definitions
- `terminal.c` - VGA text mode driver
- `bf_interpreter.c` - Brainfuck runtime interpreter
- `boot.asm` - Multiboot-compliant bootloader
- `linker.ld` - Linker script

## Future Enhancements

- File system to load Brainfuck programs from disk
- Cranium compiler support (compile Cranium to Brainfuck at runtime)
- Keyboard input for interactive Brainfuck programs
- System calls for Brainfuck programs
- Memory management for multiple Brainfuck programs

## Philosophy

Like TempleOS, this OS runs everything in a single language (Brainfuck) at runtime. No separate binaries, no user/kernel mode separation - everything is compiled and executed directly in memory, all in ring 0.
