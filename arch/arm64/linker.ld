ENTRY(_start)

MEMORY
{
    RAM : ORIGIN = 0x40080000, LENGTH = 128M
}

SECTIONS
{
    . = 0x40080000;

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

