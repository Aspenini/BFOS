ENTRY(_start)

MEMORY
{
    RAM : ORIGIN = 0x8000, LENGTH = 32M
}

SECTIONS
{
    . = 0x8000;

    .text :
    {
        *(.text.boot)
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }
}

