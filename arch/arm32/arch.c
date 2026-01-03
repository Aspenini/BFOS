/* ARM32 Architecture Implementation */

#include "../../arch.h"
#include "../../kernel.h"

static display_info_t display_info;
static boot_info_t boot_info;

/* Early architecture initialization */
void arch_early_init(void) {
    /* ARM-specific early init (e.g., set up exception vectors) */
}

/* Architecture initialization */
void arch_init(void) {
    /* Initialize display info for framebuffer */
    /* QEMU versatilepb provides framebuffer at 0x28000000 */
    /* For real hardware, this would come from device tree/ATAGs */
    display_info.buffer = (void*)0x28000000;
    display_info.width = 1024;  /* Typical framebuffer width */
    display_info.height = 768;   /* Typical framebuffer height */
    display_info.bpp = 32;       /* 32-bit RGB */
    display_info.pitch = 1024 * 4; /* width * bytes per pixel */
    
    /* Initialize UART for serial console */
    extern void uart_initialize(void);
    uart_initialize();
}

/* Memory management */
void* arch_get_framebuffer(void) {
    return display_info.buffer;
}

size_t arch_get_framebuffer_size(void) {
    return display_info.width * display_info.height * 2;
}

/* I/O operations - ARM doesn't have x86-style I/O ports */
/* These would need to be implemented via memory-mapped I/O */
uint8_t arch_inb(uint16_t port) {
    /* ARM uses memory-mapped I/O, not port I/O */
    (void)port;
    return 0;
}

void arch_outb(uint16_t port, uint8_t value) {
    (void)port;
    (void)value;
    /* Not applicable on ARM */
}

uint16_t arch_inw(uint16_t port) {
    (void)port;
    return 0;
}

void arch_outw(uint16_t port, uint16_t value) {
    (void)port;
    (void)value;
}

uint32_t arch_inl(uint16_t port) {
    (void)port;
    return 0;
}

void arch_outl(uint16_t port, uint32_t value) {
    (void)port;
    (void)value;
}

/* Interrupts */
void arch_enable_interrupts(void) {
    __asm__ volatile("cpsie i");
}

void arch_disable_interrupts(void) {
    __asm__ volatile("cpsid i");
}

void arch_halt(void) {
    __asm__ volatile("cpsid i");
    while (1) {
        __asm__ volatile("wfi"); /* Wait for interrupt */
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
    return INPUT_TYPE_UART; /* ARM typically uses UART/serial */
}

int arch_input_available(void) {
    extern int uart_has_data(void);
    return uart_has_data();
}

char arch_input_read(void) {
    extern char uart_read_char(void);
    return uart_read_char();
}

