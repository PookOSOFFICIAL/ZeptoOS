ENTRY(_start)

SECTIONS
{
    /* The Multiboot standard requires loading kernels at 1MB or higher */
    . = 1M;

    /* Put the Multiboot header first, followed by code */
    .text :
    {
        *(.multiboot)
        *(.text)
    }

    /* Read-only data */
    .rodata :
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data :
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}

