# Lesson 07: The keyboard

> **Goal:** keystrokes become characters, buffered until someone reads them.
> **Files:** `src/drivers/keyboard.c`
> **Checkpoint:** `make check L=7` (the harness types on a virtual keyboard)

## Why this matters

This is your first *input* device and your first **producer/consumer**
problem: an interrupt handler produces characters at unpredictable times,
and ordinary code consumes them whenever it gets around to it. The buffer
between them is a pattern you'll meet again and again (pipes, network
packets, disk queues).

## Background

### PS/2 and scancodes

The keyboard controller raises IRQ 1 whenever it has a byte for you and
holds that byte in its data port until you read it (and won't send more until
you do). The bytes are **scancodes**, not characters: they identify physical
keys. In scancode set 1 (what you'll receive by default), pressing a key
sends its "make" code and releasing it sends a "break" code. Find the
relation between them.

Shift isn't a character: it's state. Pressing it changes what later keys
mean, until it's released. There are two shift keys.

### A ring buffer

A fixed array plus a read index and a write index. The handler writes at the
write index, readers read at the read index, and both wrap around. Think
about these before you code: How do you tell "empty" from "full"? What
should happen when it's full? Which index does each side modify?

## Your mission

`keyboard_init`, `keyboard_getchar` (non-blocking) and `keyboard_read`
(blocking), per `include/drivers/keyboard.h`: US layout, both shift keys,
Enter → `'\n'`, Backspace → `'\b'`, Tab → `'\t'`, a buffer of at least 64
characters. Then make `kmain` echo what you type onto the screen.

## Questions before you code

1. What is the make code of `A`? Its break code? What is the general rule?
2. Why must the IRQ handler read the data port even if it's going to ignore
   the byte?
3. Shift-press arrives, then `1` press, then `1` release, then shift-release.
   What character(s) should come out?
4. In your ring buffer, can the interrupt handler and `keyboard_getchar`
   both modify the same variable? If so, what can go wrong? If not, why is
   it safe without disabling interrupts? (Single CPU.)
5. `keyboard_read` waits for a key. What instruction should it wait with,
   and what must be true about the interrupt flag?

## Milestones

1. The IRQ handler prints raw scancodes in hex. Press and release keys and
   decode the pattern yourself.
2. Lower-case letters echo. Then shift. Then Enter and Backspace.
3. A line editor in `kmain`: type, backspace, Enter prints the line back.

## The checkpoint verifies

Real key events typed by the harness (through QEMU's monitor): letters,
digits, punctuation, both shifted and unshifted, space, Enter, Backspace,
Tab; that releases produce nothing; that 20 keystrokes typed while nobody
is reading all come out in order; and that `keyboard_read` blocks until a key
arrives.

## Hints

<details><summary>Hint 1: the translation table</summary>

Make two 128-entry tables indexed by make code: unshifted and shifted
character, 0 for "produces nothing". Fill them by looking at a scancode set 1
chart row by row. Most of the keyboard is contiguous ranges.
</details>

<details><summary>Hint 2: lost or doubled keys</summary>

If a key appears twice, you're probably treating its break code as a press.
If keys go missing under fast typing, check what your buffer does when the
write index catches up to the read index.
</details>

<details><summary>Hint 3: buffer with no locking</summary>

A classic single-producer single-consumer design: the handler only ever
writes `buf[head]` and then advances `head`; the reader only reads
`buf[tail]` and then advances `tail`. If the indices are free-running
unsigned counters (use `% size` to index), `head - tail` is always the number
of buffered characters, even across wrap-around. Make them `volatile`.
</details>

## Going further

- Caps Lock (and its LED: you can *send* commands to the keyboard).
- Ctrl combinations: make Ctrl-C produce `0x03`.
- Arrow keys arrive as two-byte sequences starting with `0xE0`. Handle them.

## Reflect

- An interrupt arriving between "check buffer empty" and `hlt` in
  `keyboard_read` could make you sleep with a key waiting. Does it matter
  here? (What wakes you next, at worst?) In lesson 11 you'll fix this class of
  bug properly. Look up why `sti; hlt` is special.

## References

- OSDev wiki: "PS/2 Keyboard" (scancode set 1 table), "8042 PS/2 Controller"
- Any "circular buffer" explanation (Wikipedia is fine)
