# Debugging a kernel

There's no debugger attached to your process, no core dump and no
segfault message. The machine just does something wrong, or resets. Here
is the toolbox, in roughly the order you'll want it.

## 1. Print

Once lesson 02 is done, `kprintf` over serial is your best friend. Serial
output survives a crash and `make check` saves it to `build/serial.log`.
Before lesson 02, poke characters into `0xB8000` or out of port `0xE9`
(QEMU's debug console; add `-debugcon stdio` via `QEMU_EXTRA`).

## 2. Read QEMU's own log

QEMU can log every interrupt and exception the CPU takes, with a full
register dump:

```sh
make run   QEMU_EXTRA="-d int,cpu_reset -D build/qemu.log"
make check L=5 QEMU_EXTRA="-d int,cpu_reset"     # check.py already logs to build/qemu.log
```

Then search the log for `check_exception`. Each line looks like

```
check_exception old: 0xffffffff new 0xe
     0: v=0e e=0002 i=0 cpl=0 IP=0008:00101234 pc=00101234 SP=0010:0010ffd0 CR2=1f000008
```

`v=` is the vector (0e = page fault), `e=` the error code, `IP=` cs:eip of the
faulting instruction, `CR2=` the faulting address. Find the instruction:

```sh
addr2line -e build/kernel.elf 0x00101234      # which source line?
objdump -d build/kernel.elf | less            # search for 101234:
```

Ignore the many `v=20` (timer) lines once interrupts are on.

### Triple faults

A fault while delivering a fault is a **double fault** (vector 8); a fault
while delivering *that* is a **triple fault**, and the CPU resets. With
`-no-reboot`, QEMU exits instead. `make check` reports it as "the CPU reset
itself". In the `-d int` log, the last few `check_exception` lines tell the
story: usually #GP or #PF → #DF → reset. Common causes, by lesson:

| lesson | usual suspect |
|--------|---------------|
| 04 | bad GDT descriptor or LGDT operand; far jump to a bad selector |
| 05 | IDT not loaded or gate wrong; stub stack imbalance before `iret` |
| 06 | PIC not remapped (timer arrives as vector 8!) |
| 09 | code, stack or page tables not mapped when paging turns on |
| 12 | TSS `esp0`/`ss0` wrong, or `ltr` missing |

## 3. gdb

```sh
make debug                      # QEMU starts paused, waiting for gdb on :1234
gdb -x tools/gdbinit build/kernel.elf     # in a second terminal
```
(`make debug-check L=N` does the same for the test kernel; use
`build/test/kernel.elf` with gdb.)

Useful commands:

```
break kmain          continue        stepi / nexti      finish
info registers       p/x $eax        x/8wx $esp         x/10i $eip
p *frame             p/x some_var    bt                 display/i $pc
break *0x101234      watch ticks     layout asm         layout regs
```

Breaking on `isr_dispatch` and printing `*frame` is the fastest way to see why
an interrupt went wrong. `x/8wx $esp` right before `iret` shows you what the
CPU is about to pop.

## 4. The QEMU monitor

For `make run`, add `QEMU_EXTRA="-monitor telnet:127.0.0.1:4444,server,nowait"`
and `telnet 127.0.0.1 4444` (or press Ctrl-Alt-2 in the QEMU window).

```
info registers    the full CPU state, including GDT/IDT/TR/CR3 and segment caches
info mem          virtual memory ranges and their permissions (lesson 09+)
info tlb          every virtual → physical page mapping
info pic          8259 state: masks, pending, in service (lesson 06)
x /4wx 0xaddr     read memory through the current page tables
xp /4wx 0xaddr    read PHYSICAL memory
sendkey a         type a key
```

## 5. Shrink the problem

- Bisect with prints or `panic("got here")`: move it forward until the bug
  appears.
- Write a tiny test in `kmain` that exercises just the broken function.
- Re-read the relevant section of the Intel manual. Seriously: half of all
  kernel bugs are a misread bit.

## 6. Classic mistakes worth checking first

- A missing `volatile` on memory that hardware or an interrupt handler changes.
- Inline asm with missing clobbers (`"memory"`, `"cc"`, a register).
- An interrupt-handler stub that pushes one thing and pops another.
- Interrupts enabled (or disabled) when you assumed the opposite. Print EFLAGS.
- Integer overflow in address arithmetic (`ptr + len` wrapping past 4 GiB).
- Off by one page: `end` inclusive vs exclusive.
