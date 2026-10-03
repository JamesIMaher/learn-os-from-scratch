# Lesson 01: Booting into C

> **Goal:** the bootloader jumps to your code, and your code gets into a C
> function with a valid stack and the bootloader's information in hand.
> **Files:** `src/boot/boot.asm`, `src/kernel/main.c`, `linker.ld` (read it)
> **Checkpoint:** `make check L=1`

## Why this matters

C makes assumptions: that there is a stack, that `esp` points into it, that
the stack is aligned, that functions get their arguments in a particular
place. When the bootloader jumps to you, *none* of those are guaranteed.
The few instructions you write here create the world C needs to exist.

## Background

### What state is the CPU in?

The Multiboot specification (section 3.2, "Machine state") tells you
exactly what you can rely on when your entry point runs. Read it. Among
other things: you're in 32-bit protected mode, paging is off, interrupts are
off, and two registers hold something important. Note what it says about
`ESP`.

### The Multiboot header

How does a bootloader know a file is a kernel it may load? It searches the
start of the file for a magic number. The Multiboot header is three 32-bit
fields:

| field | meaning |
|-------|---------|
| magic | a fixed value the spec defines |
| flags | which features you are asking the bootloader for |
| checksum | a value such that magic + flags + checksum = 0 (mod 2³²) |

The flags matter later: lesson 08 needs a *memory map* from the bootloader,
and you have to ask for it here. Read section 3.1.2 and decide which bits
you want.

### Linker scripts

`linker.ld` decides where each section of your program lives in memory. You
were given it, but you should be able to explain every line. `ENTRY`, the
`.` location counter, `ALIGN`, `KEEP`, why `.multiboot` comes first, and
what `_kernel_start` / `_kernel_end` are (you'll use them in lesson 08).

### The stack and the calling convention

On x86 the stack grows *downward*: `push` decrements `esp` and then writes.
The 32-bit System V ABI ("cdecl") passes arguments **on the stack**, pushed
right-to-left, and the callee expects the stack to have been 16-byte aligned
just before the `call` instruction.

## Your mission

1. In `boot.asm`, write the Multiboot header in section `.multiboot`.
2. Reserve a 16 KiB stack in `.bss`, aligned to 16 bytes, with the labels
   `stack_bottom` and `stack_top` exported (the checkpoints look for them).
3. Write `_start`: set up the stack, call `kmain(magic, mbi)`, and if it ever
   returns, stop the CPU for good.
4. In `kmain`, prove you're alive by writing directly to video memory.

## Questions before you code

1. Which two registers carry information from the bootloader, and what's in
   each?
2. Why must `_start` be written in assembly at all? What's the first thing C
   code would do that would break right now?
3. If `kmain(a, b)` is called per cdecl, which is pushed first, `a` or `b`?
   Where is each relative to `esp` when `kmain` begins?
4. Why does the stack go in `.bss` and not `.data`? What does each choice cost
   in the size of `kernel.elf`?
5. After `kmain` returns, why isn't a single `hlt` instruction enough to stop
   forever? (What wakes a halted CPU?)

## Milestones

1. `make run` no longer complains about a Multiboot header. QEMU shows a
   black window (or its BIOS text) and sits there.
2. Your `kmain` writes a character to the top-left corner of the screen and
   you see it. (A cell at `0xB8000` is two bytes. Experiment to find out which
   byte is the character and what the other one does.)
3. `make check L=1` passes.

## The checkpoint verifies

- `kmain` is reached, with the bootloader's magic value and info pointer
  as its two arguments;
- the info includes a memory map (you asked for it);
- `stack_bottom`/`stack_top` exist, describe ≥ 8 KiB inside your kernel
  image, `stack_top` is 16-byte aligned, and your C code is running on that
  stack;
- interrupts are still off.

## Hints

<details><summary>Hint 1: the header doesn't seem to be found</summary>

Check: is the section *named* exactly `.multiboot` so the linker script
places it first? Is the checksum computed so the three fields sum to zero?
`readelf -S build/kernel.elf` shows sections and their file offsets: is
`.multiboot` within the first 8 KiB of the file? `xxd build/kernel.elf | head -300`
lets you search for your magic number (remember x86 is little-endian).
</details>

<details><summary>Hint 2: kmain gets garbage arguments</summary>

Think about the order of `push`es relative to the C prototype, and whether
anything between the bootloader's jump and your pushes overwrote the
registers you need. Put a breakpoint on `kmain` in gdb
(see `docs/debugging.md`) and `info registers` / `x/4wx $esp`.
</details>

<details><summary>Hint 3: the alignment check fails</summary>

`stack_top` aligned to 16 is not the whole story: at the moment of the
`call`, the stack must be 16-byte aligned *after* your arguments are pushed.
Count the bytes you push. You may need to adjust `esp` first.
</details>

## Debugging when stuck

- `make debug` + gdb: `break _start`, `continue`, `stepi`, `info registers`.
- If QEMU instantly exits during `make check`, the CPU probably reset
  (triple fault). `build/qemu.log` has a register dump at the moment of reset.

## Going further

- Print the bootloader's name. Multiboot info has a field for it (flag bit 9).
- Move the "proof of life" out: compute and display `_kernel_end - _kernel_start`
  in hex on the screen without any printf.

## Reflect

- Draw the stack exactly as it looks when the first instruction of `kmain`
  executes. Label every 4 bytes.
- What would happen if you forgot to reserve a stack and just called `kmain`?
  Try it. Explain what you see.

## References

- Multiboot Specification 0.6.96: sections 3.1 (header), 3.2 (machine state), 3.3 (boot info)
- System V ABI, Intel386 supplement: "Function Calling Sequence"
- NASM manual: `section`, `resb`, `align`, `global`, `extern`
- GNU ld manual: "Linker Scripts"
