# Lesson 06: Hardware interrupts and the timer

> **Goal:** devices can interrupt the CPU, and the kernel has a clock.
> **Files:** `src/arch/pic.c`, `src/drivers/timer.c`, `io_wait` in `include/arch/io.h`,
> and your dispatcher in `src/arch/idt.c`
> **Checkpoint:** `make check L=6`

## Why this matters

Until now your kernel only did things when *it* decided to. Hardware
interrupts flip that: the world decides when your code runs. The timer is
the most important one. It gives the kernel a sense of time, and in lesson
11 it becomes the heartbeat that lets the scheduler take the CPU away from a
thread that won't give it up.

This is also the first time code runs **asynchronously**: an interrupt can
fire between any two instructions of your other code. Keep that in mind; it
becomes a theme.

## Background

### The 8259 PIC

Old PCs have two cascaded 8259 Programmable Interrupt Controllers, each with
8 input lines (IRQs). The slave's output feeds the master's IRQ 2. When a
device raises its line, the PIC tells the CPU "interrupt vector N".

At power-on the master delivers IRQ 0-7 as vectors 8-15. Those are CPU
exception vectors (8 is a double fault!). So the first job is to **remap**
the PICs to vectors 32-47 with a fixed initialization sequence of four
"ICW" command words.

Each PIC also has an **interrupt mask register** (one bit per IRQ: 1 =
blocked), and needs an **End Of Interrupt** (EOI) command after each IRQ is
handled, or it will never deliver that IRQ (or any lower-priority one) again.

### The 8253/8254 PIT

The Programmable Interval Timer has an oscillator at 1193182 Hz and three
16-bit counters. Channel 0 is wired to IRQ 0. Program it with a divisor and
an operating mode and it raises IRQ 0 at oscillator ÷ divisor times per
second.

### Enabling interrupts

The `sti` instruction sets EFLAGS.IF; `cli` clears it. With IF clear, the
CPU ignores hardware interrupts (but not exceptions). Your kernel has been
running with IF clear since boot. Once you `sti`, every unmasked IRQ can
arrive at any moment.

### Waiting without spinning

`hlt` stops the CPU until the next interrupt. A loop of "check a condition,
`hlt`" sleeps efficiently. (What happens if you `hlt` with IF clear?)

## Your mission

1. `pic_init`, `pic_mask`, `pic_unmask`, `pic_send_eoi`, `irq_register`
   (contract in `include/arch/pic.h`).
2. Make your dispatcher send the EOI for vectors 32-47.
3. `timer_init(hz)`, `timer_hz`, `timer_ticks`, `timer_sleep_ms`.
4. `io_wait()`.
5. In `kmain`: PIC, timer at 100 Hz, `sti`, then print a line every second.

## Questions before you code

1. What does each ICW tell the PIC? Why is ICW3 different for master and slave?
2. IRQ 12 (PS/2 mouse) fires. Which PIC(s) need an EOI? In what order?
3. What divisor gives 100 Hz? What's the slowest rate the PIT can do?
4. `ticks` is a 64-bit counter incremented by the IRQ handler. On a 32-bit
   CPU, a 64-bit read is two 32-bit reads. Describe a sequence of events in
   which `timer_ticks()` returns a value that is wrong by about 4 billion.
   How do you prevent it?
5. Why does `ticks` need to be `volatile`?
6. `timer_sleep_ms(1)` at 100 Hz: how long can that actually take? Should it
   round up or down?

## Milestones

1. After `sti`, the kernel doesn't crash. (A missed remap means the first
   timer tick looks like a double fault.)
2. A counter on screen increases ten times per second at 10 Hz.
3. A clock that prints seconds since boot.

## The checkpoint verifies

The PIC mask registers after init and after mask/unmask, that `timer_init`
unmasks IRQ 0 without enabling interrupts, that ticks keep arriving (so EOI
works), that IRQ 0 arrives on vector 32, the tick rate measured against the
CMOS real-time clock (100 ± 15 per second), `timer_sleep_ms` accuracy, and
that `timer_ticks` never goes backwards.

## Hints

<details><summary>Hint 1: the remap sequence</summary>

The standard sequence: ICW1 "initialize, expect ICW4" to both command ports;
ICW2 the vector offset to each data port; ICW3 the cascade wiring (master:
a *bitmask* of which IRQ has a slave; slave: its cascade *identity number*);
ICW4 "8086 mode". Then write the masks you want. Put `io_wait()` between
writes for real, slow hardware.
</details>

<details><summary>Hint 2: only one tick ever arrives</summary>

The PIC is waiting for its EOI. Is your dispatcher sending it for IRQ
vectors? Are you sending it to the right port (the command port, not the data
port)? Is the value right?
</details>

<details><summary>Hint 3: the torn 64-bit read</summary>

The IRQ handler can't run while interrupts are disabled. Save EFLAGS, `cli`,
read, and restore the previous IF state. Restoring is important: blindly
`sti`-ing would enable interrupts for a caller that had them disabled.
Alternatively, read high, low, high again and retry if the high word changed.
</details>

## Going further

- Read the CMOS RTC yourself and print the wall-clock time on screen.
- Measure how long `kprintf` of one line takes, in ticks, at 1000 Hz.
- Read about the APIC and why modern systems disable the 8259 entirely.

## Reflect

- Your sleep function waits with `hlt`. What would happen if the timer IRQ
  were masked when someone called it?
- You now have code that runs "in interrupt context". What must such code
  never do? (Can it wait for a keypress?)

## References

- Intel 8259A datasheet (ICW/OCW definitions)
- Intel 8254 PIT datasheet (modes, command byte)
- OSDev wiki: "8259 PIC", "Programmable Interval Timer", "CMOS"
- Intel SDM Vol. 2: `HLT`, `CLI`, `STI`, `PUSHF`
