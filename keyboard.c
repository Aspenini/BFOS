/* PS/2 Keyboard Driver
 * Handles keyboard input via PS/2 controller
 */

#include "kernel.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Keyboard input buffer */
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static size_t keyboard_buffer_head = 0;
static size_t keyboard_buffer_tail = 0;
static size_t keyboard_buffer_count = 0;

/* Scan code to ASCII conversion (US QWERTY layout, set 1) */
static char scan_code_to_ascii[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* Read keyboard status port */
static inline uint8_t keyboard_read_status(void) {
    uint8_t status;
    __asm__ volatile("inb %1, %0" : "=a"(status) : "Nd"(KEYBOARD_STATUS_PORT));
    return status;
}

/* Read keyboard data port */
static inline uint8_t keyboard_read_data(void) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(KEYBOARD_DATA_PORT));
    return data;
}

/* Check if keyboard has data available */
static int keyboard_has_data(void) {
    return (keyboard_read_status() & 0x01) != 0;
}

/* Initialize keyboard */
void keyboard_initialize(void) {
    keyboard_buffer_head = 0;
    keyboard_buffer_tail = 0;
    keyboard_buffer_count = 0;
}

/* Handle keyboard interrupt (polling version) */
void keyboard_handle_interrupt(void) {
    if (!keyboard_has_data()) {
        return;
    }
    
    uint8_t scancode = keyboard_read_data();
    
    /* Handle key release (0xF0 prefix) */
    static int key_released = 0;
    if (scancode == 0xF0) {
        key_released = 1;
        return;
    }
    
    if (key_released) {
        key_released = 0;
        return;
    }
    
    /* Convert scan code to ASCII */
    if (scancode < 128) {
        char ascii = scan_code_to_ascii[scancode];
        if (ascii != 0 && keyboard_buffer_count < KEYBOARD_BUFFER_SIZE) {
            keyboard_buffer[keyboard_buffer_tail] = ascii;
            keyboard_buffer_tail = (keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
            keyboard_buffer_count++;
        }
    }
}

/* Get a character from keyboard buffer (non-blocking) */
int keyboard_getchar(void) {
    keyboard_handle_interrupt();
    
    if (keyboard_buffer_count == 0) {
        return -1; /* No data available */
    }
    
    char c = keyboard_buffer[keyboard_buffer_head];
    keyboard_buffer_head = (keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    keyboard_buffer_count--;
    
    return (int)(unsigned char)c;
}

/* Wait for a character from keyboard (blocking) */
char keyboard_wait_char(void) {
    int c;
    while ((c = keyboard_getchar()) == -1) {
        keyboard_handle_interrupt();
        __asm__ volatile("hlt");
    }
    return (char)c;
}
