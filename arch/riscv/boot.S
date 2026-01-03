; RISC-V Bootloader for BFOS
; Entry point for RISC-V architecture

.section .text.boot
.global _start
_start:
    ; Set up stack pointer
    la sp, stack_top
    
    ; Clear BSS section
    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    beq t0, t1, done_clear
    sw zero, 0(t0)
    addi t0, t0, 4
    j clear_bss
done_clear:
    
    ; Call kernel main
    call kernel_main
    
    ; If kernel returns, halt
    j arch_halt

.section .bss
.align 4
stack_bottom:
.space 16384 ; 16 KB
stack_top:

