# Lesson 02: Port I/O, the serial port, and a tiny libc

> **Goal:** talk to the outside world through COM1, and build the handful of
> library functions a kernel can't live without, including `printf`.
> **Files:** `include/arch/io.h`, `src/drivers/serial.c`, `src/lib/string.c`,
> `src/lib/printf.c`, `src/kernel/console.c`
> **Checkpoint:** `make check L=2`

## Why this matters

You can't debug what you can't see. The serial port is the simplest output
device there is, it works long before the screen driver does, and under
QEMU its output lands right in your terminal (or a log file) where it
survives crashes. Every lesson after this one leans on `kprintf`.

## Background

### Two address spaces

x86 has ordinary memory addresses *and* a separate 16-bit I/O port space.
Many classic PC devices live only in port space and are reached with the
`in` and `out` instructions. C has no way to say "port", so you'll write
`inb`/`outb` with GCC inline assembly. That's the first time you'll tell the
compiler precisely which registers an instruction needs.

### The 16550 UART

A UART turns bytes into a serial bit stream. COM1's registers live at ports
`0x3F8` to `0x3FF`. Several registers share an address and are selected by a
bit in another register (the "DLAB"). To use it you:

1. set the speed (a *divisor* of a 115200 Hz base clock),
2. set the frame format (8 data bits, no parity, 1 stop bit, "8N1"),
3. turn on FIFOs,
4. check the chip works, using its built-in **loopback mode** (what you send
   comes straight back),
5. then, for each byte: wait until the transmitter is ready, then write it.

Get the register map from a datasheet or the OSDev "Serial Ports" page and
work out each value yourself.

### Why `memcpy` is not optional

Even with `-ffreestanding`, GCC is allowed to emit calls to `memcpy`,
`memset`, `memmove` and `memcmp` for things like struct assignment or zeroing
a large local array. If those functions are missing you get a link error;
if they're *wrong*, you get bugs that seem to have nothing to do with them.

### `printf` is a little interpreter

A format string is a tiny program: literal characters, and `%` directives
with optional flags and a width. Your formatter walks it, pulls arguments with
`<stdarg.h>` (which works in a freestanding kernel: why?), converts numbers to
digits, pads, and emits characters. Write the core once as `kvsnprintf` and
build everything else on top.

## Your mission

1. `outb` and `inb` in `include/arch/io.h`.
2. `serial_init`, `serial_putc`, `serial_write`, `serial_can_read`,
   `serial_getc`. Read the contract in `include/drivers/serial.h`.
3. All of `include/lib/string.h`.
4. `kvsnprintf`, `ksnprintf`, `kprintf` per `include/lib/printf.h`.
5. `console_putc`/`console_write` sending to serial. (Lesson 03 adds the screen.)
6. In `kmain`, initialize serial and print something with `kprintf`.

## Questions before you code

1. In the inline asm for `outb`, why do the value and port need specific
   register constraints instead of "any register"? (Look up which operand
   forms the `out` instruction allows.)
2. What divisor gives 38400 baud? What happens if the divisor is 0?
3. Which bit of which UART register tells you the transmit holding register
   is empty? Why must you wait for it?
4. `memcpy` vs `memmove`: draw two overlapping buffers where copying
   front-to-back gives the wrong answer. Which direction is safe for which
   overlap?
5. Why do `memcmp` and `strcmp` have to compare as *unsigned* char?
6. How do you print `INT_MIN` (-2147483648) correctly? What goes wrong if you
   negate it as an `int`?
7. `ksnprintf(buf, 6, "hello world")` returns 11. Why is returning the
   *would-be* length useful to a caller?

## Milestones

1. `make run` prints "Hello from my kernel" in your terminal via `serial_write`.
2. String-function checkpoints pass (`make check L=2` shows each group).
3. `kprintf("%d %x %s\n", -42, 0xbeef, "ok")` works.
4. All lesson 02 checkpoints pass.

## The checkpoint verifies

- every `string.h` function, including overlap, signedness and `strncpy`'s odd
  padding rules;
- `kvsnprintf` conversions, flags, widths, `NULL` strings, truncation and the
  return value;
- `serial_init` returns 0, bytes actually reach COM1, and `'\n'` goes out as
  `"\r\n"`;
- `kprintf` and `console_write` reach the serial port.

## Hints

<details><summary>Hint 1: inline asm syntax</summary>

GCC's extended asm looks like
`__asm__ volatile ("instruction %0, %1" : outputs : inputs : clobbers);`.
In AT&T syntax (GCC's default) operands are reversed relative to the Intel
manual: source first, destination second. Constraint letters pick registers:
look up `"a"` and `"N"` / `"d"` in the GCC docs ("Machine Constraints", x86).
</details>

<details><summary>Hint 2: serial_init returns -1 even though output works</summary>

Loopback mode is a bit in the Modem Control Register. In loopback, write a
test byte to the data register and read the data register back. Make sure
you leave loopback mode afterwards, or nothing reaches the outside.
</details>

<details><summary>Hint 3: structuring printf</summary>

Separate "where do characters go" from "how do I format". If your formatter
takes a little "emit one character" callback (or a struct with a buffer, a
capacity and a count), `kvsnprintf` and `kprintf` become two thin wrappers
over the same code, and truncation is handled in exactly one place.
</details>

<details><summary>Hint 4: digits come out backwards</summary>

`n % 10` gives you the *last* digit first. Fill a small temporary buffer
from its end, or reverse afterwards. Unsigned arithmetic makes negatives and
hex easier: convert to unsigned before the digit loop.
</details>

## Debugging when stuck

- `make check` saves everything your kernel sent to COM1 in `build/serial.log`.
  `xxd build/serial.log` shows exactly which bytes arrived.
- Test `kvsnprintf` by printing its result *and* its return value.

## Going further

- `%lld` / `%llu` (64-bit). You'll need `libgcc`'s helpers; see what symbols
  the linker asks for if you don't link it.
- Precision: `%.3s`, `%.5d`.
- Read from the serial port: make a tiny "echo what I type" loop. Under
  `make run`, keystrokes in your terminal arrive on COM1.

## Reflect

- Your `printf` takes a variable number of arguments. How does `va_arg` know
  where the next argument is? What happens if the format string lies about
  the types?
- Why is it fine for `serial_putc` to busy-wait, but would it be a problem in
  a real kernel serving many programs?

## References

- GCC manual: "Extended Asm", "Machine Constraints"
- Intel SDM Vol. 2: `IN`, `OUT`
- 16550 UART datasheet (TI PC16550D), or OSDev wiki "Serial Ports"
- C11 standard §7.24 (string handling), §7.21.6.1 (fprintf: the formatting rules)
