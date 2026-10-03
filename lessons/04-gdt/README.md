# Lesson 04: The Global Descriptor Table

> **Goal:** replace the bootloader's segment setup with your own: kernel and
> user segments for code and data, and a slot reserved for the TSS.
> **Files:** `src/arch/gdt.c` (plus assembly, inline or a new `.asm`)
> **Checkpoint:** `make check L=4`

## Why this matters

Segmentation is mostly vestigial on modern x86: everyone uses "flat"
segments that span all 4 GiB. But the GDT still decides two things you
absolutely need: **which privilege level** code runs at (ring 0 for the
kernel, ring 3 for user programs), and where the CPU finds the TSS when it
switches between them. The bootloader left a GDT somewhere in memory you
don't own. You need your own before you can handle interrupts reliably.

## Background

### Segment registers and selectors

In protected mode, `cs`, `ds`, `ss`, `es`, `fs`, `gs` don't hold addresses.
They hold **selectors**: an index into the GDT plus two other fields. Read
Intel SDM Vol. 3A §3.4.2 and work out what the low three bits of a selector
mean. That explains why the user selectors in `include/arch/gdt.h` are
`0x1B` and `0x23`.

### Descriptors

Each GDT entry is an 8-byte descriptor holding a base address, a limit, an
access byte (present, privilege level, type) and flags (granularity, 16/32
bit). The fields are split up and scattered across the 8 bytes for
historical reasons (backwards compatibility with the 80286). Packing them
correctly is the whole exercise.

A "flat" segment: base 0, limit covering all 4 GiB. A 20-bit limit field can
only cover 4 GiB if the limit is counted in 4 KiB units: that's the
granularity flag.

### Loading it

`lgdt` tells the CPU where the table is. But the CPU caches each segment's
descriptor inside the segment register (the hidden "descriptor cache"), so
after `lgdt` nothing changes until you **reload every segment register**.
Data segment registers can be loaded with `mov`. `cs` cannot: you need an
instruction that changes `cs` and `eip` together.

## Your mission

`gdt_init()`: six descriptors in the order `include/arch/gdt.h` specifies
(null, kernel code, kernel data, user code, user data, TSS placeholder),
loaded and in effect. Call it from `kmain`.

## Questions before you code

1. What are the three fields of a selector? Decode `0x08`, `0x10`, `0x1B`,
   `0x23`, `0x28`.
2. Draw the 8-byte descriptor layout. Where do the base's 32 bits go? The
   limit's 20 bits?
3. Work out the access byte for each of the four segments by hand, bit by bit.
4. With granularity = 4 KiB, what limit value means "all 4 GiB"?
5. Which instruction reloads `cs`? Why can't `mov cs, ax` exist?
6. Why must entry 0 be all zeros? What happens if you load selector 0 into
   `ds`? Into `cs`?

## Milestones

1. `gdt_init` runs and the kernel doesn't crash. (A mistake here usually
   means an instant triple fault, so this milestone is real.)
2. In gdb, `info registers` shows `cs=0x8` and `ds=ss=0x10`.
3. The checkpoint passes. It asks the CPU itself to decode your descriptors
   (`lar` and `lsl` instructions), so a pass means the hardware agrees.

## Hints

<details><summary>Hint 1: struct layout</summary>

Model the descriptor as a packed struct of the pieces exactly as they lie in
memory: a 16-bit field, another 16-bit field, then single bytes. Write one
helper `set_entry(index, base, limit, access, flags)` that slices base and
limit into place, and keep all bit-twiddling there.
</details>

<details><summary>Hint 2: reloading CS</summary>

A *far jump* (`jmp selector:offset`) loads both. In GCC inline asm the AT&T
spelling is `ljmp $SEL, $label`; in NASM it's `jmp SEL:label`. Jump to the
very next instruction.
</details>

<details><summary>Hint 3: triple fault right after lgdt</summary>

Check the `lgdt` operand: it's a 6-byte structure (16-bit limit = size − 1,
then 32-bit base), not a pointer to the table. Is it packed? Also, run
`make debug`, break after `lgdt`, and in the QEMU monitor (`Ctrl-Alt-2` in
a QEMU window, or `-monitor stdio`) run `info registers`: it shows the GDT
base/limit and the decoded segment caches.
</details>

## Going further

- Look up what the 'accessed' bit is and watch the CPU set it.
- Read about how x86-64 long mode ignores most of these fields, and what's left.

## Reflect

- You set every segment to base 0 and limit 4 GiB. Then what is
  segmentation *doing* for you? What will provide memory protection
  instead? (Lesson 09.)
- The TSS slot is empty for now. Guess what the CPU will need from it once
  user programs run. Check your guess in lesson 12.

## References

- Intel SDM Vol. 3A, Chapter 3: §3.4.2 (selectors), §3.4.5 (descriptors), §3.4.3 (segment registers)
- Intel SDM Vol. 2: `LGDT`, `LAR`, `LSL`, `JMP` (far)
- OSDev wiki: "GDT", "GDT Tutorial" (concepts; write your own code)
