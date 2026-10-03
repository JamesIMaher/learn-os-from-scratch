/* user.ld - how user programs are laid out (lesson 13). Programs start at the
 * bottom of user space. Compare with the kernel's linker.ld. */
ENTRY(_start)

SECTIONS
{
    . = 0x40000000;
    .text   : ALIGN(4K) { *(.text.start) *(.text .text.*) }
    .rodata : ALIGN(4K) { *(.rodata .rodata.*) }
    .data   : ALIGN(4K) { *(.data .data.*) }
    .bss    : ALIGN(4K) { *(COMMON) *(.bss .bss.*) }
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame) }
}
