# Lesson 00: Setup, and what "freestanding" means

> **Goal:** a working toolchain, and a clear picture of how source files turn
> into something a CPU can boot.
> **Checkpoint:** `make` succeeds and `make check L=1` runs (and fails: you
> haven't written lesson 01 yet).

## Why this matters

Every program you've written so far ran *on top of* an operating system. The
OS loaded it, gave it a stack, handed it `argv`, provided `printf` through
libc, and cleaned up when it exited. None of that exists now. You are
building the thing that provides it. The first step is a compiler setup
that assumes nothing.

## Install the tools

You need: a C compiler that can produce 32-bit x86 ELF code, a linker,
`nasm`, `qemu-system-i386`, `make`, `python3`, and (strongly recommended)
`gdb`.

**Linux (Debian/Ubuntu)**
```sh
sudo apt install build-essential gcc-multilib nasm qemu-system-x86 gdb python3
```
Your normal `gcc -m32` works here. `gcc-multilib` provides the 32-bit
`libgcc`.

**macOS**
```sh
brew install i686-elf-gcc i686-elf-binutils nasm qemu
# gdb: brew install i386-elf-gdb (or use lldb's gdb-remote)
```
The Makefile automatically prefers `i686-elf-gcc` when it exists. You *need*
a cross compiler on macOS because Apple's toolchain emits Mach-O, not ELF.

**Windows**: use WSL2 with Ubuntu and follow the Linux steps.

Check:
```sh
nasm -v
qemu-system-i386 --version
make            # builds build/kernel.elf (with a warning about _start: expected)
make check L=1  # should say QEMU could not boot the kernel: expected!
```

## Background

### Hosted vs. freestanding

The C standard describes two kinds of environments. A *hosted* one has the
full standard library and a `main`. A *freestanding* one guarantees only a
few headers that are pure compile-time definitions: `<stdint.h>`,
`<stddef.h>`, `<stdbool.h>`, `<stdarg.h>`, `<limits.h>`, `<float.h>`,
`<iso646.h>`, `<stdalign.h>`, `<stdnoreturn.h>`. No `printf`, no `malloc`,
no `memcpy`. A kernel is freestanding.

### The build pipeline

```
 .c  --(gcc -c)-->  .o  \
                          >--(ld + linker.ld)-->  kernel.elf  --(QEMU -kernel)-->  running
 .asm --(nasm)--->  .o  /
```

QEMU's `-kernel` option contains a small bootloader that understands the
**Multiboot** standard. It reads `kernel.elf`, copies its segments to the
physical addresses the ELF file asks for, and jumps to the entry point. You
write that entry point in lesson 01.

### The test kernel

`make check` builds a *second* kernel: your code plus `tests/`. Your
`kmain` is renamed out of the way and the test runner's `kmain` takes its
place. It brings your subsystems up in order and checks them, reporting over
a debug port that doesn't depend on your drivers. `tools/check.py` boots it
in QEMU, collects the results, types keystrokes when a test asks, and checks
your serial output.

## Your mission

Nothing to code yet. Instead, read the `Makefile` and answer the questions
below in `notes/00-setup.md`.

## Questions to answer

1. What does each of these `CFLAGS` do, and why would a kernel need it?
   `-ffreestanding`, `-nostdlib`, `-fno-builtin`, `-fno-stack-protector`,
   `-fno-pic`, `-mno-sse` (hint for that one: who saves the SSE registers
   when an interrupt arrives?), `-m32`.
2. The Makefile links against `libgcc`. What is in it, and why might a
   32-bit kernel need it even with no standard library? (Try: what
   instruction would divide a 64-bit number on a 32-bit CPU?)
3. Run `make` and then `readelf -h -l build/kernel.elf`. What's the entry
   point address? Why does the linker warn about `_start`?
4. In `linker.ld`, why might the kernel start at 1 MiB rather than at
   address 0? (Look up the "PC memory map below 1 MiB".)
5. Why does `make check` pass `-no-reboot` to QEMU?

## Going further

- Read `tools/check.py`. It's short. Figure out how it knows a test failed.
- Install a cross compiler even on Linux (OSDev wiki "GCC Cross-Compiler").
  What subtle problems does a cross compiler prevent?

## References

- GCC manual: "Options for Code Generation", "C Dialect Options"
- OSDev wiki: "Why do I need a Cross Compiler?" (concept page)
- Multiboot specification, sections 1-3
