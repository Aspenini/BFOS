/* Configuration Management
 * Handles system configuration settings like resolution
 */

#include "kernel.h"

/* Configuration structure */
typedef struct {
    size_t vga_width;
    size_t vga_height;
} config_t;

/* Default configuration */
static config_t system_config = {
    .vga_width = 80,
    .vga_height = 25
};

/* Initialize configuration */
void config_initialize(void) {
    /* Use defaults for now */
    system_config.vga_width = 80;
    system_config.vga_height = 25;
}

/* Get current VGA width */
size_t config_get_vga_width(void) {
    return system_config.vga_width;
}

/* Get current VGA height */
size_t config_get_vga_height(void) {
    return system_config.vga_height;
}

/* Set VGA resolution */
int config_set_resolution(size_t width, size_t height) {
    /* Validate common resolutions */
    if ((width == 80 && height == 25) ||
        (width == 80 && height == 50) ||
        (width == 132 && height == 25) ||
        (width == 132 && height == 43) ||
        (width == 132 && height == 50)) {
        system_config.vga_width = width;
        system_config.vga_height = height;
        return 0; /* Success */
    }
    return -1; /* Invalid resolution */
}

/* Get resolution as string (for display) */
void config_get_resolution_string(char* buffer, size_t max_len __attribute__((unused))) {
    size_t i = 0;
    size_t width = system_config.vga_width;
    size_t height = system_config.vga_height;
    
    /* Convert width to string */
    if (width >= 100) {
        buffer[i++] = '0' + (width / 100);
        width %= 100;
    }
    if (width >= 10 || system_config.vga_width >= 100) {
        buffer[i++] = '0' + (width / 10);
        width %= 10;
    }
    buffer[i++] = '0' + width;
    
    buffer[i++] = 'x';
    
    /* Convert height to string */
    if (height >= 100) {
        buffer[i++] = '0' + (height / 100);
        height %= 100;
    }
    if (height >= 10 || system_config.vga_height >= 100) {
        buffer[i++] = '0' + (height / 10);
        height %= 10;
    }
    buffer[i++] = '0' + height;
    buffer[i] = '\0';
}

