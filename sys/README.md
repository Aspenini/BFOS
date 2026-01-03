# System Files Directory

This directory contains files that are automatically embedded into the BFOS filesystem when the kernel is built.

## Structure

- `components/` - Brainfuck programs that can be executed as commands
  - Files here are accessible as commands (e.g., `components/hello.bf` can be run as `hello`)

## How It Works

1. When you run `make`, the build system runs `build_sysfs.py`
2. The script scans this `sys/` directory and all subdirectories
3. It generates `sysfs_data.c` which contains all files as C string literals
4. The kernel calls `sysfs_initialize()` at boot to create the filesystem structure

## Adding Files

Simply add `.bf` files (or any files) to the appropriate subdirectory:

```bash
# Add a new Brainfuck program
echo "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>." > sys/components/myprogram.bf

# Rebuild
make
```

The file will automatically be included in the filesystem!

## Notes

- Hidden files (starting with `.`) are ignored
- Directory structure is preserved
- Files are embedded at compile time, so you must rebuild after adding files

