/* Terminal/VGA text mode driver */

#include "kernel.h"

/* VGA entry helper */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

/* Terminal state */
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
static char cursor_char = '_';
static int cursor_visible = 1;

/* Get current VGA width (helper) */
static size_t get_vga_width(void) {
    return config_get_vga_width();
}

/* Get current VGA height (helper) */
static size_t get_vga_height(void) {
    return config_get_vga_height();
}

/* Remove all cursor characters from screen (except current position) */
static void terminal_cleanup_cursors(void) {
    size_t width = get_vga_width();
    size_t height = get_vga_height();
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            const size_t index = y * width + x;
            uint16_t current = terminal_buffer[index];
            if ((current & 0xFF) == cursor_char) {
                /* Only clear if it's not the current cursor position */
                if (y != terminal_row || x != terminal_column) {
                    terminal_buffer[index] = vga_entry(' ', (current >> 8) & 0xFF);
                }
            }
        }
    }
}

/* Initialize terminal */
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry(COLOR_LIGHT_GREY, COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    size_t width = get_vga_width();
    size_t height = get_vga_height();
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            const size_t index = y * width + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    /* Make sure no cursor characters are left behind */
    cursor_visible = 0;
}

/* Set terminal color */
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

/* Put character at position */
static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    size_t width = get_vga_width();
    const size_t index = y * width + x;
    terminal_buffer[index] = vga_entry(c, color);
}

/* Scroll terminal up by one line */
static void terminal_scroll(void) {
    size_t width = get_vga_width();
    size_t height = get_vga_height();
    
    /* Shift all lines up by one */
    for (size_t y = 1; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            const size_t src_index = y * width + x;
            const size_t dst_index = (y - 1) * width + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }
    
    /* Clear the bottom line */
    for (size_t x = 0; x < width; x++) {
        const size_t index = (height - 1) * width + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

/* Handle newline */
void terminal_putchar(char c) {
    size_t width = get_vga_width();
    size_t height = get_vga_height();
    
    /* Clear cursor at old position before writing */
    if (cursor_visible) {
        const size_t old_index = terminal_row * width + terminal_column;
        uint16_t old = terminal_buffer[old_index];
        if ((old & 0xFF) == cursor_char) {
            /* Restore the character that was under the cursor, or space */
            terminal_buffer[old_index] = vga_entry(' ', (old >> 8) & 0xFF);
        }
    }
    
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row >= height) {
            terminal_scroll();
            terminal_row = height - 1;
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == width) {
            terminal_column = 0;
            if (++terminal_row >= height) {
                terminal_scroll();
                terminal_row = height - 1;
            }
        }
    }
    
    /* Show cursor at new position */
    if (cursor_visible) {
        terminal_update_cursor();
    }
}

/* Write string */
void terminal_writestring(const char* data) {
    size_t datalen = 0;
    while (data[datalen] != '\0') {
        datalen++;
    }
    for (size_t i = 0; i < datalen; i++)
        terminal_putchar(data[i]);
}

/* Get terminal row */
size_t terminal_get_row(void) {
    return terminal_row;
}

/* Get terminal column */
size_t terminal_get_column(void) {
    return terminal_column;
}

/* Set terminal position */
void terminal_set_position(size_t x, size_t y) {
    size_t width = get_vga_width();
    size_t height = get_vga_height();
    
    /* Clear cursor at old position */
    if (cursor_visible) {
        const size_t old_index = terminal_row * width + terminal_column;
        uint16_t old = terminal_buffer[old_index];
        if ((old & 0xFF) == cursor_char) {
            terminal_buffer[old_index] = vga_entry(' ', (old >> 8) & 0xFF);
        }
    }
    
    if (x < width && y < height) {
        terminal_column = x;
        terminal_row = y;
    }
    
    /* Show cursor at new position */
    if (cursor_visible) {
        terminal_update_cursor();
    }
}

/* Update cursor display (solid, no blinking) */
void terminal_update_cursor(void) {
    if (cursor_visible) {
        /* Clean up any stray cursors first */
        terminal_cleanup_cursors();
        
        size_t width = get_vga_width();
        const size_t index = terminal_row * width + terminal_column;
        uint16_t current = terminal_buffer[index];
        uint8_t color = (current >> 8) & 0xFF;
        
        /* Always show cursor (solid, no blinking) */
        terminal_buffer[index] = vga_entry(cursor_char, color);
    }
}

/* Hide cursor */
void terminal_hide_cursor(void) {
    cursor_visible = 0;
    size_t width = get_vga_width();
    const size_t index = terminal_row * width + terminal_column;
    uint16_t current = terminal_buffer[index];
    if ((current & 0xFF) == cursor_char) {
        terminal_buffer[index] = vga_entry(' ', (current >> 8) & 0xFF);
    }
}

/* Show cursor */
void terminal_show_cursor(void) {
    cursor_visible = 1;
    terminal_update_cursor();
}

/* Clear terminal screen */
void terminal_clear(void) {
    size_t width = get_vga_width();
    size_t height = get_vga_height();
    
    /* Hide cursor while clearing */
    int was_visible = cursor_visible;
    if (cursor_visible) {
        terminal_hide_cursor();
    }
    
    /* Clear entire screen and remove any stray cursor characters */
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            const size_t index = y * width + x;
            uint16_t current = terminal_buffer[index];
            /* If it's a cursor character, clear it */
            if ((current & 0xFF) == cursor_char) {
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            } else {
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
        }
    }
    
    /* Reset cursor position */
    terminal_row = 0;
    terminal_column = 0;
    
    /* Restore cursor visibility */
    if (was_visible) {
        terminal_show_cursor();
    }
}

/* Switch VGA mode (sets resolution) */
void terminal_set_resolution(size_t width, size_t height) {
    /* Set the resolution in config */
    if (config_set_resolution(width, height) == 0) {
        /* Clear screen and reinitialize */
        terminal_clear();
        terminal_initialize();
    }
}

