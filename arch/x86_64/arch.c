/* x86_64 Architecture Implementation */

#include "../../arch.h"
#include "../../kernel.h"

static display_info_t display_info;
static boot_info_t boot_info;

/* Early architecture initialization (before C runtime) */
void arch_early_init(void) {
    /* Stack is already set up by boot.asm */
    /* Nothing else needed for x86_64 */
}

/* Architecture initialization */
void arch_init(void) {
    /* Initialize display info for VGA text mode */
    display_info.buffer = (void*)VGA_MEMORY;
    display_info.width = 80;
    display_info.height = 25;
    display_info.bpp = 16;  /* 16 bits per character (8 char + 8 color) */
    display_info.pitch = 160; /* 80 chars * 2 bytes */
}

/* Memory management */
void* arch_get_framebuffer(void) {
    return (void*)VGA_MEMORY;
}

size_t arch_get_framebuffer_size(void) {
    return 80 * 25 * 2; /* 80x25, 2 bytes per character */
}

/* I/O operations */
uint8_t arch_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void arch_outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint16_t arch_inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void arch_outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t arch_inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void arch_outl(uint16_t port, uint32_t value) {
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

/* Interrupts */
void arch_enable_interrupts(void) {
    __asm__ volatile("sti");
}

void arch_disable_interrupts(void) {
    __asm__ volatile("cli");
}

void arch_halt(void) {
    __asm__ volatile("cli");
    while (1) {
        __asm__ volatile("hlt");
    }
}

/* Boot information */
boot_info_t* arch_get_boot_info(void) {
    return &boot_info;
}

/* Display information */
display_info_t* arch_get_display_info(void) {
    return &display_info;
}

/* Input abstraction */
input_type_t arch_get_input_type(void) {
    return INPUT_TYPE_PS2; /* x86 uses PS/2 keyboard */
}

int arch_input_available(void) {
    /* Check PS/2 keyboard status port */
    uint8_t status = arch_inb(0x64);
    return (status & 0x01) != 0;
}

char arch_input_read(void) {
    /* Read from PS/2 keyboard data port */
    return (char)arch_inb(0x60);
}

