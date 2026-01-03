; ARM32 Bootloader for BFOS
; Entry point for ARM32 architecture

.section .text.boot
.global _start
_start:
    ; Set up stack pointer
    ; Stack typically starts at high memory
    ldr sp, =stack_top
    
    ; Clear BSS section
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
clear_bss:
    cmp r0, r1
    beq done_clear
    str r2, [r0], #4
    b clear_bss
done_clear:
    
    ; Call kernel main
    bl kernel_main
    
    ; If kernel returns, halt
    b arch_halt

.section .bss
.align 4
stack_bottom:
.space 16384 ; 16 KB
stack_top:

