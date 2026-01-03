/* RISC-V Architecture Implementation */

#include "../../arch.h"
#include "../../kernel.h"

static display_info_t display_info;
static boot_info_t boot_info;

/* Early architecture initialization */
void arch_early_init(void) {
    /* RISC-V-specific early init */
}

/* Architecture initialization */
void arch_init(void) {
    /* Initialize display info for framebuffer */
    /* QEMU virt machine provides framebuffer */
    display_info.buffer = (void*)0x80000000; /* Common RISC-V framebuffer address */
    display_info.width = 1024;
    display_info.height = 768;
    display_info.bpp = 32;
    display_info.pitch = 1024 * 4;
    
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

/* I/O operations - RISC-V uses memory-mapped I/O */
uint8_t arch_inb(uint16_t port) {
    (void)port;
    return 0;
}

void arch_outb(uint16_t port, uint8_t value) {
    (void)port;
    (void)value;
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
    __asm__ volatile("csrsi mstatus, 8"); /* Set MIE bit */
}

void arch_disable_interrupts(void) {
    __asm__ volatile("csrci mstatus, 8"); /* Clear MIE bit */
}

void arch_halt(void) {
    __asm__ volatile("csrci mstatus, 8"); /* Disable interrupts */
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
    return INPUT_TYPE_UART; /* RISC-V typically uses UART */
}

int arch_input_available(void) {
    extern int uart_has_data(void);
    return uart_has_data();
}

char arch_input_read(void) {
    extern char uart_read_char(void);
    return uart_read_char();
}

