/* Brainfuck Interpreter Module
 * Core runtime execution engine for Brainfuck programs
 * This is the heart of the OS - all user code runs through this
 */

#include "kernel.h"

/* VGA entry helper */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

#define TAPE_SIZE 30000

/* Brainfuck tape - global state */
static uint8_t bf_tape[TAPE_SIZE];
static size_t bf_pointer = 0;

/* Reset Brainfuck interpreter state */
void bf_reset(void) {
    for (size_t i = 0; i < TAPE_SIZE; i++) {
        bf_tape[i] = 0;
    }
    bf_pointer = 0;
}

/* Execute Brainfuck code from memory */
void bf_execute(const char* code) {
    const char* pc = code;  /* Program counter */
    size_t depth;
    
    /* Reset tape for new execution */
    bf_reset();
    
    /* Execute Brainfuck program */
    while (*pc != '\0') {
        switch (*pc) {
            case '>':
                if (bf_pointer < TAPE_SIZE - 1) bf_pointer++;
                break;
                
            case '<':
                if (bf_pointer > 0) bf_pointer--;
                break;
                
            case '+':
                bf_tape[bf_pointer]++;
                break;
                
            case '-':
                bf_tape[bf_pointer]--;
                break;
                
            case '.':
                terminal_putchar(bf_tape[bf_pointer]);
                break;
                
            case ',':
                /* Input from keyboard */
                {
                    int c = keyboard_getchar();
                    if (c == -1) {
                        keyboard_handle_interrupt();
                        c = keyboard_getchar();
                    }
                    bf_tape[bf_pointer] = (c == -1) ? 0 : (uint8_t)c;
                }
                break;
                
            case '[':
                if (bf_tape[bf_pointer] == 0) {
                    /* Skip to matching ] */
                    depth = 1;
                    pc++;
                    while (depth > 0 && *pc != '\0') {
                        if (*pc == '[') depth++;
                        if (*pc == ']') depth--;
                        if (depth > 0) pc++;
                    }
                }
                break;
                
            case ']':
                if (bf_tape[bf_pointer] != 0) {
                    /* Jump back to matching [ */
                    depth = 1;
                    pc--;
                    while (depth > 0 && pc >= code) {
                        if (*pc == ']') depth++;
                        if (*pc == '[') depth--;
                        if (depth > 0) pc--;
                    }
                }
                break;
                
            default:
                /* Ignore non-BF characters (comments) */
                break;
        }
        pc++;
    }
}

/* Load and execute a Brainfuck program from memory */
void bf_load_and_run(const char* bf_code) {
    terminal_setcolor(vga_entry(COLOR_LIGHT_CYAN, COLOR_BLACK));
    terminal_writestring("[BF] Executing...\n");
    terminal_setcolor(vga_entry(COLOR_LIGHT_GREEN, COLOR_BLACK));
    
    bf_execute(bf_code);
    
    terminal_putchar('\n');
}

/* Get current tape pointer (for debugging) */
size_t bf_get_pointer(void) {
    return bf_pointer;
}

/* Get tape value at pointer */
uint8_t bf_get_value(void) {
    return bf_tape[bf_pointer];
}

