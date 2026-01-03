/* Architecture Abstraction Layer
 * Provides portable interfaces for architecture-specific operations
 */

#ifndef ARCH_H
#define ARCH_H

#include "kernel.h"

/* Architecture detection */
#if defined(__x86_64__) || defined(__i386__)
    #define ARCH_X86
    #if defined(__x86_64__)
        #define ARCH_X86_64
    #else
        #define ARCH_X86_32
    #endif
#elif defined(__arm__) || defined(__aarch64__)
    #define ARCH_ARM
    #if defined(__aarch64__)
        #define ARCH_ARM64
    #else
        #define ARCH_ARM32
    #endif
#elif defined(__riscv)
    #define ARCH_RISCV
    #if __riscv_xlen == 64
        #define ARCH_RISCV64
    #else
        #define ARCH_RISCV32
    #endif
#else
    #error "Unsupported architecture"
#endif

/* Architecture-specific initialization */
void arch_early_init(void);
void arch_init(void);

/* Memory management */
void* arch_get_framebuffer(void);
size_t arch_get_framebuffer_size(void);

/* I/O operations */
uint8_t arch_inb(uint16_t port);
void arch_outb(uint16_t port, uint8_t value);
uint16_t arch_inw(uint16_t port);
void arch_outw(uint16_t port, uint16_t value);
uint32_t arch_inl(uint16_t port);
void arch_outl(uint16_t port, uint32_t value);

/* Interrupts */
void arch_enable_interrupts(void);
void arch_disable_interrupts(void);
void arch_halt(void);

/* Boot information */
typedef struct {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
    /* Add more fields as needed for different boot protocols */
} boot_info_t;

boot_info_t* arch_get_boot_info(void);

/* Terminal/Display abstraction */
typedef struct {
    void* buffer;
    size_t width;
    size_t height;
    size_t bpp;  /* Bits per pixel */
    size_t pitch; /* Bytes per line */
} display_info_t;

display_info_t* arch_get_display_info(void);

/* Input abstraction */
typedef enum {
    INPUT_TYPE_NONE = 0,
    INPUT_TYPE_PS2,
    INPUT_TYPE_UART,
    INPUT_TYPE_VIRTIO
} input_type_t;

input_type_t arch_get_input_type(void);
int arch_input_available(void);
char arch_input_read(void);

#endif /* ARCH_H */

