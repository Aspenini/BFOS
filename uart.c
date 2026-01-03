/* UART Driver
 * Provides serial console input/output for ARM/RISC-V
 */

#include "kernel.h"
#include "arch.h"

/* UART base addresses (common QEMU addresses) */
#define UART_PL011_BASE 0x09000000  /* ARM Versatile PB */
#define UART_NS16550_BASE 0x10000000 /* RISC-V virt */

/* UART register offsets */
/* PL011 (ARM) registers */
#define UART_DR     0x00  /* Data register */
#define UART_RSR    0x04  /* Receive status register */
#define UART_FR     0x0C  /* Flag register */
#define UART_IBRD   0x24  /* Integer baud rate divisor */
#define UART_FBRD   0x28  /* Fractional baud rate divisor */
#define UART_LCR_H  0x2C  /* Line control register */
#define UART_CR     0x30  /* Control register */
#define UART_IMSC   0x38  /* Interrupt mask set/clear */
#define UART_ICR    0x44  /* Interrupt clear register */

/* NS16550 (RISC-V) registers */
#define UART_RBR    0x00  /* Receiver buffer (read) */
#define UART_THR    0x00  /* Transmitter holding (write) */
#define UART_IER    0x01  /* Interrupt enable */
#define UART_IIR    0x02  /* Interrupt identification */
#define UART_FCR    0x02  /* FIFO control */
#define UART_LCR    0x03  /* Line control */
#define UART_MCR    0x04  /* Modem control */
#define UART_LSR    0x05  /* Line status */
#define UART_MSR    0x06  /* Modem status */
#define UART_SCR    0x07  /* Scratch */
#define UART_DLL    0x00  /* Divisor latch low (when DLAB=1) */
#define UART_DLM    0x01  /* Divisor latch high (when DLAB=1) */

/* UART flag bits */
#define UART_FR_TXFF (1 << 5)  /* Transmit FIFO full */
#define UART_FR_RXFE (1 << 4)  /* Receive FIFO empty */
#define UART_FR_BUSY (1 << 3)  /* UART busy */

/* Get UART base address based on architecture */
static volatile void* get_uart_base(void) {
    #if defined(ARCH_ARM32) || defined(ARCH_ARM64)
        return (volatile void*)UART_PL011_BASE;
    #elif defined(ARCH_RISCV)
        return (volatile void*)UART_NS16550_BASE;
    #else
        return (volatile void*)0;
    #endif
}

/* Read UART register */
static inline uint32_t uart_read_reg(volatile void* base, uint32_t offset) {
    volatile uint32_t* reg = (volatile uint32_t*)((uint8_t*)base + offset);
    return *reg;
}

/* Write UART register */
static inline void uart_write_reg(volatile void* base, uint32_t offset, uint32_t value) {
    volatile uint32_t* reg = (volatile uint32_t*)((uint8_t*)base + offset);
    *reg = value;
}

/* Initialize UART */
void uart_initialize(void) {
    volatile void* uart_base = get_uart_base();
    if (!uart_base) return;
    
    #if defined(ARCH_ARM32) || defined(ARCH_ARM64)
        /* PL011 UART initialization */
        /* Disable UART */
        uart_write_reg(uart_base, UART_CR, 0);
        
        /* Set baud rate (115200) */
        uart_write_reg(uart_base, UART_IBRD, 1);
        uart_write_reg(uart_base, UART_FBRD, 40);
        
        /* Set line control: 8 bits, 1 stop bit, no parity */
        uart_write_reg(uart_base, UART_LCR_H, 0x70);
        
        /* Enable UART, TX, RX */
        uart_write_reg(uart_base, UART_CR, 0x301);
    #elif defined(ARCH_RISCV)
        /* NS16550 UART initialization */
        /* Set DLAB bit to access divisor registers */
        uart_write_reg(uart_base, UART_LCR, 0x80); /* DLAB = 1 */
        /* Set baud rate divisor (115200 at 10MHz = ~87) */
        uart_write_reg(uart_base, UART_DLL, 87 & 0xFF);  /* Divisor low */
        uart_write_reg(uart_base, UART_DLM, (87 >> 8) & 0xFF);  /* Divisor high */
        /* Clear DLAB, set 8N1 */
        uart_write_reg(uart_base, UART_LCR, 0x03); /* 8N1, DLAB = 0 */
        /* Enable FIFO */
        uart_write_reg(uart_base, UART_FCR, 0x01); /* Enable FIFO */
    #endif
}

/* Check if UART has data available */
int uart_has_data(void) {
    volatile void* uart_base = get_uart_base();
    if (!uart_base) return 0;
    
    #if defined(ARCH_ARM32) || defined(ARCH_ARM64)
        /* PL011: Check receive FIFO empty flag */
        uint32_t fr = uart_read_reg(uart_base, UART_FR);
        return !(fr & UART_FR_RXFE);
    #elif defined(ARCH_RISCV)
        /* NS16550: Check line status register */
        uint32_t lsr = uart_read_reg(uart_base, UART_LSR);
        return (lsr & 0x01) != 0; /* Data ready (DR bit) */
    #else
        return 0;
    #endif
}

/* Read a character from UART */
char uart_read_char(void) {
    volatile void* uart_base = get_uart_base();
    if (!uart_base) return 0;
    
    #if defined(ARCH_ARM32) || defined(ARCH_ARM64)
        /* PL011: Read data register */
        return (char)(uart_read_reg(uart_base, UART_DR) & 0xFF);
    #elif defined(ARCH_RISCV)
        /* NS16550: Read receiver buffer register */
        return (char)(uart_read_reg(uart_base, UART_RBR) & 0xFF);
    #else
        return 0;
    #endif
}

/* Write a character to UART */
void uart_write_char(char c) {
    volatile void* uart_base = get_uart_base();
    if (!uart_base) return;
    
    #if defined(ARCH_ARM32) || defined(ARCH_ARM64)
        /* PL011: Wait for transmit FIFO to be not full */
        while (uart_read_reg(uart_base, UART_FR) & UART_FR_TXFF) {
            /* Wait */
        }
        uart_write_reg(uart_base, UART_DR, (uint32_t)c);
    #elif defined(ARCH_RISCV)
        /* NS16550: Wait for transmit holding register empty */
        while (!(uart_read_reg(uart_base, UART_LSR) & 0x20)) { /* THRE bit */
            /* Wait */
        }
        uart_write_reg(uart_base, UART_THR, (uint32_t)c);
    #endif
}

/* Write a string to UART */
void uart_write_string(const char* str) {
    while (*str) {
        uart_write_char(*str++);
    }
}

