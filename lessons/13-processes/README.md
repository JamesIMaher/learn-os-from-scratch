# Lesson 13: Processes and ELF

> **Goal:** compile real C programs, load them from ELF files into fresh
> address spaces, run them as processes, and collect their exit status.
> **Files:** `src/proc/elf.c`, `src/proc/process.c`, `SYS_WAIT` in
> `src/proc/syscall.c`, and user space: `user/lib/crt0.asm`,
> `user/lib/syscalls.c`, `user/lib/ustring.c`, `user/lib/uprintf.c`
> **Checkpoint:** `make check L=13`

## Why this matters

Hand-assembled blobs were enough to prove ring 3 works. Real programs are
compiled, linked, have data and zeroed memory, need a C runtime and a C
library, and are stored in an executable file format. This lesson makes your
kernel able to run *programs*.

## Background

### ELF

An ELF executable starts with a header (magic number, class, machine,
entry point, where the **program headers** are). Each program header of type
`PT_LOAD` says: take `p_filesz` bytes at file offset `p_offset` and place them
at virtual address `p_vaddr`, in a region `p_memsz` bytes long. The bytes from
`p_filesz` to `p_memsz` are zero. That's where `.bss` comes from. Look at one:

```sh
make user
readelf -h -l build/user/bin/hello.elf
objdump -d build/user/bin/hello.elf | less
```

The loader runs in the kernel and the file is untrusted. Every offset, size
and address in it must be validated before use.

### Processes

A process bundles an address space, a main thread, a pid, and a parent.
Spawning: create an address space, load the ELF into it, map a user stack,
start a user thread at the entry point. Waiting: the parent blocks until the
child's thread exits, takes its status, and cleans up. A process can't free
its own address space while running in it, so the waiter does.

### The user-side runtime

`crt0.asm` provides `_start`: the real entry point that calls `main` and
passes its return value to `exit`. `user/lib` is a tiny C library: system call
wrappers (inline asm around `int 0x80`), string functions, and `printf`
writing to fd 1. You've written all of these before, in the kernel. User
programs link only against `user/lib`, never against kernel code.

## Your mission

1. `elf_load`: validate and load an ELF32 i386 executable (`include/proc/process.h`).
2. `process_spawn_image` and `process_wait`, plus `SYS_WAIT`.
   `SYS_GETPID` now returns the process id.
3. The user runtime and library in `user/`.

## Questions before you code

1. Which fields of the ELF header must you check to know you have a 32-bit,
   little-endian, i386 *executable*?
2. A `PT_LOAD` segment has `p_vaddr = 0x40001010`, `p_memsz = 0x2000`. Which
   pages must be mapped? What if two segments share a page?
3. List every way a malicious ELF could make a naive loader read or write
   outside where it should. (There are at least five.)
4. You copy segment bytes into pages of an address space that *isn't*
   loaded in CR3. How do you write to them? (Lesson 09 set this up for you.)
5. `main` returns 7. Trace the path of that 7 until it comes out of
   `process_wait`.
6. A process exits while its parent hasn't called wait yet. What must you
   keep, and what can you free immediately?

## Milestones

1. `readelf` on `hello.elf`, then print the same information from your
   kernel by parsing the module in memory.
2. `elf_load` + a user thread runs `hello`. It prints. (You need `crt0`,
   `write`, `getpid` and `printf`.)
3. Exit status flows back through `process_wait`.

## The checkpoint verifies

`elf_load` accepts a real program and maps its entry point as user memory;
it rejects junk, truncated files, the wrong machine type, an entry point in
kernel space, and an out-of-bounds program header table; `hello` runs and
exits with status 7 printing its real pid; `elfcheck` verifies `.data`,
`.bss` across several pages, `.rodata`, a 40 KiB stack, `snprintf` and
`getpid` from inside the program; a crashing program is killed while the
kernel carries on; `process_wait` works exactly once per child and rejects
non-children; six concurrent processes waited for in reverse order; and 50
spawn/wait cycles without leaking memory.

## Hints

<details><summary>Hint 1: loading segments</summary>

Validate *everything* first, before mapping anything. Then for each
`PT_LOAD`: for every page from `p_vaddr` rounded down to `p_vaddr + p_memsz`,
make sure a zeroed, user-writable frame is mapped. Then copy `p_filesz` bytes,
translating each destination address with `vmm_translate` (through the
identity map you can write to the physical address directly).
</details>

<details><summary>Hint 2: crt0</summary>

`_start` can be four instructions: align the stack, call `main`, push the
result, call `exit`. `exit` never returns, but put a loop after it anyway.
</details>

<details><summary>Hint 3: one wrapper for all system calls</summary>

```
static inline int syscall3(int n, int a, int b, int c)
```
with inline asm that puts `n` in `eax`, `a`/`b`/`c` in `ebx`/`ecx`/`edx`,
executes `int $0x80`, returns `eax`, and clobbers `"memory"`. Every wrapper
is one line on top of it.
</details>

<details><summary>Hint 4: who frees what</summary>

The child's thread exits normally (it can't free its own kernel stack or
address space). The waiting parent joins the thread (freeing the thread and
its stack), then destroys the address space and frees the process record.
</details>

## Going further

- Command-line arguments: copy `argv` strings onto the new user stack and
  pass `argc`/`argv` to `main`.
- `sbrk` and a user-space `malloc`.
- `fork()`: copy an entire address space. Then copy-on-write using the page
  fault handler.

## Reflect

- What does your kernel trust about an ELF file now, after all the checks?
- `hello` and `elfcheck` both live at `0x40000000`. How can both run at
  the same time?

## References

- System V ABI, Chapter 4 (Object Files) and 5 (Program Loading); i386 supplement
- `man 5 elf`
- OSDev wiki: "ELF", "ELF Tutorial" (concepts)
