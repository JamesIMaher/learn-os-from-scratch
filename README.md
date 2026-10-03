# Learn OS From Scratch

Build a small but real operating system for 32-bit x86, in C and a little
assembly, one working piece at a time. Bare metal, no libraries.

By the end you will have written a kernel that boots, handles interrupts,
manages physical and virtual memory, runs preemptively scheduled threads,
loads ELF programs into isolated user-mode processes, serves system calls,
reads files from a filesystem, and runs a shell you can type into.

**This course does not give you the code.** Each lesson hands you:

- **a goal**, and why it matters;
- **background** on the concepts and the hardware, explained in prose;
- **a contract**: header files declaring exactly what to build, with
  stub `.c` files waiting for you;
- **questions** to answer *before* you write code;
- **milestones** that let you see progress early;
- **automated checkpoints** (`make check L=N`) that tell you whether it works,
  without telling you how to make it work;
- **hints**, folded away and tiered from a gentle nudge to a strong push, for
  when you're truly stuck;
- **"going further"** ideas and reflection questions.

You write every line of the kernel. You will get stuck, read the Intel
manual, triple-fault the CPU, stare at register dumps, and then figure it
out. That's the course working as intended.

## Roadmap

| #  | Lesson | You build | Checkpoint |
|----|--------|-----------|------------|
| 00 | [Setup](lessons/00-setup/README.md) | a toolchain, and an understanding of the build | `make` builds |
| 01 | [Booting into C](lessons/01-boot/README.md) | Multiboot header, stack, `_start` → `kmain` | `make check L=1` |
| 02 | [Port I/O, serial, a tiny libc](lessons/02-serial-and-libc/README.md) | `inb`/`outb`, UART driver, `memcpy`…, `printf` | `make check L=2` |
| 03 | [VGA text mode](lessons/03-vga/README.md) | a screen driver with scrolling and a cursor | `make check L=3` |
| 04 | [The GDT](lessons/04-gdt/README.md) | segment descriptors, privilege levels | `make check L=4` |
| 05 | [Interrupts and exceptions](lessons/05-interrupts/README.md) | IDT, assembly stubs, a C dispatcher, panic | `make check L=5` |
| 06 | [The PIC and the timer](lessons/06-pic-and-timer/README.md) | hardware IRQs, a clock | `make check L=6` |
| 07 | [The keyboard](lessons/07-keyboard/README.md) | scancodes → characters, an input buffer | `make check L=7` |
| 08 | [Physical memory](lessons/08-physical-memory/README.md) | a page-frame allocator from the BIOS memory map | `make check L=8` |
| 09 | [Paging](lessons/09-paging/README.md) | page tables, address spaces, page faults | `make check L=9` |
| 10 | [The kernel heap](lessons/10-heap/README.md) | `kmalloc` / `kfree` | `make check L=10` |
| 11 | [Threads and scheduling](lessons/11-threads/README.md) | context switches, a preemptive scheduler, sleep, mutexes | `make check L=11` |
| 12 | [User mode and system calls](lessons/12-user-mode/README.md) | ring 3, the TSS, `int 0x80`, fault isolation | `make check L=12` |
| 13 | [Processes and ELF](lessons/13-processes/README.md) | a program loader, processes, a user C library | `make check L=13` |
| 14 | [A filesystem](lessons/14-filesystem/README.md) | a tar-based read-only filesystem, file descriptors | `make check L=14` |
| 15 | [A shell](lessons/15-shell/README.md) | the first program you'll actually *use* | `make check L=15` |
| 16 | [Where next](lessons/16-where-next/README.md) | — | — |

Expect each lesson to take an evening to a weekend. Lessons 05, 09, 11 and
12 are the big ones.

## How to work through a lesson

1. Read the lesson's README top to bottom **before** touching code.
2. Answer the "Questions before you code" in your own words. Write them down
   in `notes/` (there's a [template](notes/TEMPLATE.md)). If you can't answer
   one, that's where to go reading.
3. Open the header(s) named in the lesson. They are the contract: they say
   *what* each function must do, never *how*.
4. Work through the milestones. Run `make run` constantly and look at what
   happens.
5. Run `make check L=N` (with N = the lesson). Checkpoints are cumulative: they
   re-test lessons 1..N each time, so a later change that breaks an earlier
   lesson gets caught.
6. Only open a hint after you've been stuck for a while and tried the
   debugging tips. Open them one at a time.
7. Do the "Reflect" questions. They're how you find out whether you
   understood it or just got it working.

### The rules you set for yourself

- Reading references is encouraged: the Intel manuals, the OSDev wiki's
  *concept* pages, datasheets, specs, `man` pages.
- Copying someone else's kernel code defeats the purpose. If you look at
  another OS's source, close it before you write yours.
- The files in `tests/` are a specification. Reading what a test *checks* is
  fair; a few tests must poke hardware to observe your work, so the plumbing
  around the checks contains small spoilers.

## Quick start

```sh
# 1. install the toolchain (see lessons/00-setup)
# 2. then:
make            # builds build/kernel.elf (it won't boot yet: that's lesson 01)
make check L=1  # runs the lesson 01 checkpoints (they fail until you do lesson 01)
```

## Repository layout

```
lessons/        one README per lesson: start here
include/        the contracts: header files declaring what you build
src/            YOUR kernel: stubs with TODO(lesson NN) markers
  boot/           the first instructions after the bootloader
  arch/           x86-specific: GDT, IDT, interrupt stubs, PIC
  drivers/        serial, VGA, timer, keyboard
  lib/            string functions, printf
  kernel/         kmain, console, panic
  mm/             physical memory, paging, heap
  proc/           threads, scheduler, system calls, processes, ELF loader
  fs/             the filesystem
user/           YOUR user space: C runtime, user C library, the shell
tests/          the checkpoint tests (and the programs they run)
tools/          check.py (the test harness), gdb setup
docs/           setup, debugging guide, references
notes/          your learning journal
```

## Design choices, and why

- **32-bit x86 (i386).** Much simpler to boot than x86-64 (no long mode
  setup, two-level paging), and every concept transfers directly. Lesson 16
  sketches the move to 64-bit.
- **Multiboot + QEMU `-kernel`.** No bootloader to write or install, so you
  start in protected mode, in C, on day one. (Writing a bootloader is a
  great project of its own; see lesson 16.)
- **NASM** for assembly: Intel syntax, matching the Intel manuals.
- **Identity-mapped kernel.** The kernel sees physical memory at the same
  addresses, which keeps paging understandable. Real kernels usually live in
  the "higher half"; that's a lesson 16 project.
- **Serial port first.** Your first driver is the one that lets you see
  what's going on.

## Help, I'm stuck

- [docs/debugging.md](docs/debugging.md): gdb, QEMU's monitor, reading
  triple faults, the techniques that matter.
- [docs/references.md](docs/references.md): where to look things up.
