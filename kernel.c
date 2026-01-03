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
    terminal_initialize();
    
    /* Boot message */
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREEN, COLOR_BLACK));
    terminal_writestring("BFOS - Brainfuck Operating System v1.0\n");
    terminal_writestring("Runtime Brainfuck Compilation System\n");
    terminal_writestring("=====================================\n\n");
    
    /* Welcome message - printed directly */
    terminal_setcolor(vga_entry(COLOR_YELLOW, COLOR_BLACK));
    terminal_writestring("Hello, this is BFOS - Brainfuck Operating System!\n\n");
    
    /* Now demonstrate Brainfuck execution */
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("Brainfuck Runtime Compiler Ready.\n");
    terminal_writestring("All user code runs in Brainfuck, compiled at runtime.\n\n");
    
    /* Sample Brainfuck program: prints "Hello, World!" */
    const char* hello_world_bf = 
        "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++."
        ">+.+++++++..+++.>++.<<+++++++++++++++.>."
        "+++.------.--------.>+.>.";
    
    bf_load_and_run(hello_world_bf);
    
    terminal_putchar('\n');
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK));
    terminal_writestring("\nSystem ready. All code runs in Brainfuck (ring 0).\n");
    terminal_writestring("No binaries - just Brainfuck source compiled at runtime.\n");
    
    /* Infinite loop to keep kernel running */
    for (;;) {
        __asm__ volatile("hlt");
    }
}
