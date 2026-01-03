; ARM64 Bootloader for BFOS
; Entry point for ARM64 architecture

.section .text.boot
.global _start
_start:
    ; Set up stack pointer
    ldr x0, =stack_top
    mov sp, x0
    
    ; Clear BSS section
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    mov x2, #0
clear_bss:
    cmp x0, x1
    beq done_clear
    str x2, [x0], #8
    b clear_bss
done_clear:
    
    ; Call kernel main
    bl kernel_main
    
    ; If kernel returns, halt
    b arch_halt

.section .bss
.align 8
stack_bottom:
.space 16384 ; 16 KB
stack_top:

