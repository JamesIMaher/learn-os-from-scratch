# Lesson 16: Where next

You have a kernel with interrupts, memory management, preemptive
multitasking, protected user processes, system calls, a filesystem and a
shell. Every real OS is built on these same ideas, just with more of them.
Here are directions to take it, roughly from smallest to largest. Each is a
project you can scope the same way as the lessons: write the contract
first, then the tests, then the code.

## Polish what you have

- **Kernel stack traces** on panic, with symbol names (parse the ELF symbol
  table: QEMU can load it as a module).
- **A `ps` and a `free`**: system calls that expose thread and memory stats.
- **Sleep properly**: make `read(0)` block on a wait queue woken by the
  keyboard IRQ instead of halting in a loop.
- **Locking audit**: find every piece of shared kernel state and write down
  what protects it.

## Bigger features

- **Pipes and redirection** (`ls | cat`, `> file`): needs a kernel pipe
  buffer, `dup2`, and blocking on both ends.
- **`fork` and `exec`**: duplicate an address space; then **copy-on-write**
  using the page fault handler.
- **`sbrk`/`mmap`** and a user `malloc`.
- **Signals**: Ctrl-C kills the foreground program.
- **A writable filesystem**: a ramfs first, then a real disk.
- **Demand paging and swapping**: lesson 09's page-fault trick, applied for real.

## New hardware

- **ATA PIO disk driver**, then **FAT** or **ext2**.
- **PCI enumeration**, then a network card (RTL8139 or e1000 in QEMU), then
  ARP, IP, UDP, a toy TCP.
- **Framebuffer graphics** via the multiboot video mode fields; draw text
  with a bitmap font.
- **APIC and SMP**: start the other CPU cores. Every lock you thought was
  fine needs rethinking.

## Rebuild the foundations

- **Your own bootloader**: a 512-byte boot sector that loads your kernel from
  disk and switches to protected mode. You'll understand real mode, the A20
  line, and why Multiboot exists.
- **Higher-half kernel**: kernel at `0xC0000000`, user space below it.
- **x86-64**: long mode, four-level paging, `syscall`/`sysret`, a new ABI.
  Everything you learned transfers; the details change.
- **Another architecture**: RISC-V under QEMU is beautifully simple and
  well documented. Port your kernel and see which parts were x86 and which
  were *operating system*.

## Reading for the road

- *Operating Systems: Three Easy Pieces*: Arpaci-Dusseau (free online)
- *xv6: a simple, Unix-like teaching operating system*: MIT's book and
  source. Read it now, after you've built your own; you'll recognize
  every design decision and see a few you'd make differently.
- *Operating Systems: Design and Implementation*: Tanenbaum (MINIX)
- The OSDev wiki and forums
- The Intel SDM Vol. 3, front to back, once more. It reads differently now.
