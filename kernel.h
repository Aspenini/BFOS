/* Kernel header file
 * Common definitions and function declarations
 */

#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/* VGA text mode color constants */
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_LIGHT_GREY 7
#define COLOR_DARK_GREY 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW 14
#define COLOR_WHITE 15

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int size_t;
typedef unsigned long uint32_t;

/* VGA entry helper - must be defined in each file that uses it */
/* Use: vga_entry(character, color) */

/* Terminal functions */
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_writestring(const char* data);

/* Brainfuck interpreter functions */
void bf_reset(void);
void bf_execute(const char* code);
void bf_load_and_run(const char* bf_code);
size_t bf_get_pointer(void);
uint8_t bf_get_value(void);

#endif

