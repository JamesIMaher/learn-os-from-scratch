# Lesson 05: Interrupts and exceptions

> **Goal:** when the CPU hits an exception or an `int N` instruction, it lands
> in *your* code, which saves the full machine state, calls a C handler, and
> resumes exactly where it left off.
> **Files:** `src/arch/idt.c`, `src/arch/isr.asm`, `src/kernel/panic.c`
> **Checkpoint:** `make check L=5`

## Why this matters

Interrupts are how the outside world (timers, keyboards, disks) gets the
CPU's attention, how the CPU reports errors (divide by zero, bad memory
access), and how user programs will ask the kernel for help (system calls).
This is the most important piece of plumbing in the kernel. Almost
every later lesson plugs a handler into what you build here.

Until now, any CPU exception meant a silent triple fault and a reboot. After
this lesson you get a panic message with registers instead.

## Background

### The IDT

The Interrupt Descriptor Table has up to 256 **gate descriptors**, one per
vector. Each gate says: which code segment and offset to jump to, what kind
of gate it is, and the lowest privilege level allowed to trigger it with an
`int` instruction. `lidt` loads it, much like `lgdt`.

Vectors 0-31 are reserved for CPU exceptions (#DE divide error = 0,
#GP general protection = 13, #PF page fault = 14, ...). You'll put hardware
IRQs at 32-47 next lesson, and system calls at `0x80`.

### What the CPU does on an interrupt

It pushes `eflags`, `cs`, `eip` (and, coming from ring 3, `ss` and `esp`
first) onto the stack, *for some exceptions* pushes an **error code**,
clears the interrupt flag (for interrupt gates) and jumps to the handler. The
handler ends with `iret`, which pops it all back. Read Intel SDM Vol. 3A
§6.12 until you can draw the stack.

### Why assembly stubs?

A C function can't be an interrupt handler: it would clobber registers the
interrupted code was using, it returns with `ret` instead of `iret`, and it
doesn't know which vector fired. So each vector gets a tiny assembly entry
that normalizes the stack (fake error code where the CPU didn't push one,
plus the vector number), then all of them jump to one common stub that saves
everything, calls your C dispatcher with a pointer to the saved state, and
restores it all.

The saved state is `struct interrupt_frame` in `include/arch/idt.h`. That
layout is a contract: your stubs must produce exactly it. And because the
dispatcher gets a *pointer* into the stack, a handler can **change** the
saved registers, and the change takes effect on `iret`. System calls will
return values this way.

## Your mission

1. Gate descriptors, a 256-entry IDT, `lidt`.
2. Entry stubs for vectors 0-47 and `0x80`, a common stub, and a C
   dispatcher that calls the handler registered with `isr_register`.
3. Unhandled exceptions call `panic` with a useful register dump.
4. `panic()`: print and halt forever.
5. Vector `0x80` must be callable from ring 3 (you'll see why in lesson 12).

## Questions before you code

1. Draw an interrupt gate's 8 bytes. What's the "type" for a 32-bit
   interrupt gate vs. a trap gate? What's the difference in behavior?
2. Which exceptions push an error code? (Make a table from SDM §6.15. Get
   this wrong and the stack is off by 4 bytes on exactly those vectors.)
3. The struct lists `gs, fs, es, ds` first and `eip, cs, eflags` near the end.
   Why does the first-pushed field come *last*?
4. Your common stub has to load the kernel's data segments before calling C.
   Why? (What might `ds` hold if the interrupt came from user mode?)
5. After the C dispatcher returns, how many bytes must you discard before
   `iret`, and why?
6. `eip` in the frame for `int3` points *after* the instruction. For `#GP` it
   points *at* the faulting instruction. Why the difference? What does that
   mean for returning from a fault you didn't fix?

## Milestones

1. `int $3` from `kmain` prints "breakpoint" from your handler and then
   `kmain` keeps going.
2. A deliberate divide by zero gives a panic with a register dump instead of
   a reboot.
3. All checkpoints pass.

## The checkpoint verifies

The IDT is loaded with room for `0x80`; `isr_register` swaps handlers;
`int3` delivers vector 3, error code 0, the right `cs`/`ds`/`eip`, and resumes;
all six general registers arrive in the frame and **modifications to the
frame are restored**; `#DE` arrives on vector 0; a `#GP` delivers the right
error code and segment registers survive; 1000 interrupts in a row work
(no stack leak).

## Hints

<details><summary>Hint 1: generating 49 stubs</summary>

NASM macros: one macro for "no error code" stubs (push a dummy 0, then the
vector), one for "error code" stubs (push only the vector), and `%rep` /
`%assign` to stamp them out. Also emit a table of stub addresses so the C
side can fill the IDT in a loop.
</details>

<details><summary>Hint 2: passing the frame to C</summary>

After the common stub has pushed everything, `esp` *is* the address of the
`struct interrupt_frame`. Push it as the argument (cdecl), call the
dispatcher, then pop the argument off.
</details>

<details><summary>Hint 3: "general registers survive" fails</summary>

`pushad`/`popad` (NASM: `pusha`/`popa` in 32-bit mode) save and restore all
eight general registers in one go, in exactly the order the struct expects.
Check that the *segment* pushes happen after `pushad` and the pops in the
reverse order. And that nothing between the C call and `popad` overwrites a
register you want restored.
</details>

<details><summary>Hint 4: the #GP test triple-faults</summary>

If the #GP handler isn't registered correctly, the CPU can't deliver the
exception... and then a second fault happens while delivering it (#DF,
vector 8), and if *that* fails, it resets. `make debug-check L=5`, then in
gdb break on your dispatcher and inspect the frame (`p *frame`).
`build/qemu.log` will show `check_exception` lines when you add
`-d int` to QEMU's flags.
</details>

## Debugging when stuck

`docs/debugging.md` has a section on reading QEMU's `-d int` output: every
exception with vector, error code, and the CPU state. It's the single most
useful tool for this lesson.

## Going further

- Print a stack backtrace in `panic` by walking the saved `ebp` chain
  (that's why the Makefile uses `-fno-omit-frame-pointer`).
- Look up the double-fault (#DF) exception and why a robust kernel runs its
  handler on a separate stack via a task gate.

## Reflect

- Interrupt gates clear IF; trap gates don't. Which would you want for a
  system call? For a timer interrupt? Note your answers; revisit in lesson 12.
- What happens if an exception occurs *inside* your common stub before it
  has finished saving state?

## References

- Intel SDM Vol. 3A, Chapter 6: §6.10 (IDT), §6.11 (descriptors), §6.12 (handling), §6.15 (exception reference)
- Intel SDM Vol. 2: `LIDT`, `IRET`, `PUSHA`/`POPA`, `INT n`
- NASM manual: "Macros", `%rep`
