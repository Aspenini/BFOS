; x86_32 Bootloader for BFOS
; Multiboot-compliant bootloader (32-bit)

MAGIC_NUMBER equ 0x1BADB002
FLAGS        equ 0x0
CHECKSUM     equ -(MAGIC_NUMBER + FLAGS)

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KB
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Clear direction flag
    cld
    
    ; Call kernel main
    extern kernel_main
    call kernel_main
    
    ; If kernel returns, halt
    cli
.hang:
    hlt
    jmp .hang
.end:

