/* BFOS - Brainfuck Operating System Kernel
 * Runtime Brainfuck compilation and execution system
 * Similar to TempleOS/HolyC - everything runs in Brainfuck, compiled at runtime
 */

#include "kernel.h"

/* VGA entry helper */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

/* Kernel main entry point */
void kernel_main(void) {
    /* Architecture-specific initialization */
    extern void arch_early_init(void);
    extern void arch_init(void);
    arch_early_init();
    arch_init();
    
    config_initialize();
    terminal_initialize();
    keyboard_initialize();
    fs_initialize();
    
    /* Boot message */
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREEN, COLOR_BLACK));
    terminal_writestring("BFOS - Brainfuck Operating System\n");
    terminal_writestring("Runtime Brainfuck Compilation System\n");
    terminal_writestring("=====================================\n\n");
    
    /* Welcome message */
    terminal_setcolor(vga_entry(COLOR_YELLOW, COLOR_BLACK));
    terminal_writestring("Initializing file system...\n");
    
    /* Create system directory structure */
    fs_mkdir("sys");
    fs_chdir("sys");
    
    /* Load files from sys/ directory */
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("Loading system files...\n");
    sysfs_initialize();
    
    fs_chdir("/");
    
    /* Create example directory and file */
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("Creating example files...\n");
    
    fs_mkdir("kevinapps");
    fs_chdir("kevinapps");
    const char* hello_bf = "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.";
    fs_create_file("hello.bf", hello_bf);
    fs_chdir("/"); 
    
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREEN, COLOR_BLACK));
    terminal_writestring("System ready!\n\n");
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("Type 'help' for available commands.\n");
    terminal_writestring("All commands are Brainfuck programs in /sys/components/\n\n");
    
    /* Start shell */
    shell_main();
}
