# Lesson 03: VGA text mode

> **Goal:** a screen driver: colored text, newlines, tabs, backspace,
> wrapping, scrolling, and a blinking cursor that follows along.
> **Files:** `src/drivers/vga.c`, `src/kernel/console.c`
> **Checkpoint:** `make check L=3`

## Why this matters

This is your first **memory-mapped** device: you control it by writing to
ordinary-looking memory addresses instead of I/O ports. It's also your
first piece of kernel *state* (where is the cursor? what color?) that every
later lesson will touch whenever it prints.

## Background

### The text buffer

In 80×25 text mode, the VGA hardware continuously reads a 4000-byte array
starting at physical address `0xB8000` and draws it. Each of the 2000 cells
is a 16-bit value: one byte is the character (code page 437), the other is
an **attribute** byte holding a foreground and background color, 4 bits each.
Lesson 01 had you poke this memory. Now work out the exact layout of the
attribute byte from a reference, and which byte of the 16-bit cell is which
(remember x86 is little-endian).

### `volatile`

The compiler assumes memory only changes when your program changes it, and
that writes nobody reads back can be dropped or merged. Neither is true for
device memory. `volatile` tells the compiler every access is observable and
must actually happen.

### The hardware cursor

The blinking underline is drawn by the CRT controller (CRTC), not by you. It
has dozens of internal registers reached through an index/data port pair:
write a register number to the index port, then read or write the data port.
Two of those registers hold the cursor's cell index (high and low byte).

### Scrolling

When the cursor moves past the last row, every row moves up one, the top row
is lost, and the new bottom row is blank. That's a memory move of 24 rows.

## Your mission

Implement everything in `include/drivers/vga.h`. Then make `console_putc`
write to *both* serial and the screen, so every `kprintf` appears in both
places.

## Questions before you code

1. Write the attribute byte for "yellow on blue". Which bit, if set, might
   make text blink instead of giving a bright background? (Look it up.)
2. What 16-bit value is a light-grey-on-black space? (The checkpoint expects
   exactly this after `vga_init`.)
3. Given row `r` and column `c`, what's the cell index? The byte address?
4. Which CRTC registers hold the cursor position, and through which two ports?
5. When you scroll, is it safe to copy rows with `memcpy`? (Source and
   destination overlap.)
6. Tab: from column 3, where do you land? From column 8? From column 79?

## Milestones

1. `vga_init` clears the screen. `vga_write("Hello")` shows text.
2. Colors work: draw a little palette of all 16 foreground colors.
3. Print 30 numbered lines and watch it scroll.
4. The cursor blinks right after the last character you printed.
5. `kprintf` output appears on screen *and* in the terminal.

## The checkpoint verifies

It reads video memory and the CRTC cursor registers directly:
clearing (with the current color), character and attribute placement,
`\n` `\r` `\t` `\b` behavior (including `\b` at column 0), wrapping at column
80, scrolling (content moves up, new row blank in the current color, repeated
scrolling), the hardware cursor position after every step, and that
`console_write` reaches the screen.

## Hints

<details><summary>Hint 1: nothing appears / wrong colors</summary>

Print one cell with a value you construct by hand, like `0x2F41`, then look:
which part became the letter and which the color? Then build a small helper
that makes a cell from a character and the current attribute so you never
compute it inline again.
</details>

<details><summary>Hint 2: when exactly to scroll</summary>

There are two reasonable designs: scroll *eagerly* when the cursor moves off
the bottom, or *lazily* just before writing into row 25. The checkpoint
checks the cursor after writing `"\nZ"` at the bottom row, and the
cursor must always be on screen. Decide which design keeps the cursor
on screen at all times.
</details>

<details><summary>Hint 3: the cursor doesn't move</summary>

The CRTC index port is `0x3D4` and the data port is `0x3D5`. The cursor
location is split across two registers, `0x0E` (high byte) and `0x0F` (low
byte). Update the hardware after every logical cursor change; it's cheap
enough.
</details>

## Going further

- Hide the cursor, or change its shape (CRTC registers 0x0A/0x0B).
- Support a few ANSI escape sequences (`\033[31m` for red) so the same
  output looks right on serial and screen.
- Keep a scrollback buffer in memory and let a key scroll up (after lesson 07).

## Reflect

- Your console now writes to two devices. What happens if two pieces of code
  print at the same time? (You can't trigger this yet. Lesson 06 makes it
  possible, and lesson 11 makes it likely.)

## References

- OSDev wiki: "Text UI", "Text Mode Cursor", "Printing To Screen"
- FreeVGA project: CRT Controller registers
- Code page 437 character table
