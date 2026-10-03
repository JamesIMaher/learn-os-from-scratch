; =============================================================================
; boot.asm - the very first code that runs.                        [Lesson 01]
;
; The bootloader (QEMU's -kernel loader, or GRUB) has loaded this ELF file
; into memory and will jump to the symbol `_start` (see ENTRY in linker.ld)
; in 32-bit protected mode, with paging off, interrupts off, and NO STACK
; you can trust. Your job is to get from here into C.
;
; TODO(lesson 01): write, in this file,
;
;   1. A Multiboot 1 header in a section called .multiboot. The bootloader
;      only finds it if it is within the first 8 KiB of the file and aligned
;      to 4 bytes; linker.ld puts .multiboot first. It is three 32-bit
;      fields: magic, flags, checksum. Which flags do you want? (You need
;      the memory map later. Do you need page-aligned modules?)
;
;   2. A stack: reserve some space (16 KiB is plenty) in .bss, aligned to
;      16 bytes. Export two labels `stack_bottom` and `stack_top` (with
;      `global`) - the checkpoint tests look for them.
;
;   3. `_start` (exported with `global`), which must:
;        - point ESP at the stack. Which end? Which way does x86 grow stacks?
;        - call kmain(magic, mbi) - your C function in src/kernel/main.c.
;          The bootloader left `magic` in EAX and `mbi` in EBX. How does the
;          32-bit x86 C calling convention (cdecl) pass arguments?
;        - if kmain ever returns: disable interrupts and halt forever.
;          Why is a single HLT not enough?
;
; NASM syntax reference: https://www.nasm.us/doc/
; =============================================================================

bits 32

; extern kmain
; global _start, stack_bottom, stack_top

section .multiboot
    ; TODO(lesson 01)

section .bss
    ; TODO(lesson 01)

section .text
    ; TODO(lesson 01)
