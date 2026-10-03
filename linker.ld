/*
 * Kernel linker script.
 *
 * The bootloader (QEMU's built-in multiboot loader, or GRUB) reads the ELF
 * program headers and copies each segment to the *physical* address given
 * here. There is no virtual memory yet, so virtual == physical.
 *
 * Lesson 01 asks you to explain every line of this file. Don't skip that.
 */
ENTRY(_start)

SECTIONS
{
    . = 1M;
    _kernel_start = .;

    .text BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.multiboot))
        *(.text .text.*)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data .data.*)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    . = ALIGN(4K);
    _kernel_end = .;

    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame) }
}
