# Lesson 12: User mode and system calls

> **Goal:** run code in ring 3, where it can't touch the kernel, can't run
> privileged instructions, and can only ask the kernel for things through
> system calls. And when it misbehaves, only *it* dies.
> **Files:** `tss_init`/`tss_set_kernel_stack` in `src/arch/gdt.c`,
> `thread_create_user` in `src/proc/thread.c`, `src/proc/syscall.c`,
> and your exception path (`src/arch/idt.c`, the #PF handler)
> **Checkpoint:** `make check L=12`

## Why this matters

This is the line between "a program" and "an operating system". Everything
so far ran with full privileges. From now on there's a trust boundary: user
code is assumed buggy or hostile, and the kernel must survive anything it
does.

## Background

### Getting down to ring 3

There's no "go to ring 3" instruction. The only way to *lower* privilege is
to return from an interrupt: `iret` pops `eip`, `cs`, `eflags`, and, if the
new `cs` has a lower privilege, also `esp` and `ss`. So you build that stack
by hand, with user selectors (RPL 3), and `iret` into the user program.

### Getting back up: the TSS

When an interrupt arrives while the CPU is in ring 3, it can't use the
user's stack (it might be garbage, or malicious). It switches to a kernel
stack, and it finds that stack's address in the **Task State Segment**:
the `ss0`/`esp0` fields. The TSS is referenced by a descriptor in the GDT
(slot 5) and loaded with `ltr`. Every thread has its own kernel stack, so on
every switch the scheduler must update `esp0`.

### System calls

User code executes `int 0x80` with arguments in registers (see
`include/abi/syscall_nums.h`, the ABI contract). Your handler reads them from
the interrupt frame, does the work, and stores the result in the frame's
`eax`, which `iret` restores into the user's `eax`. The gate's DPL must allow
ring 3 to use `int 0x80` (you set that up in lesson 05).

### Never trust a pointer

`write(1, buf, len)` hands you an address. It might point into the kernel,
at unmapped memory, or wrap around the end of the address space. Every
pointer from user space must be checked before the kernel dereferences
it: entirely inside user space, mapped, user-accessible, and writable if the
kernel will write to it. That's `user_range_ok`.

### Faults in user mode

A user program that executes `cli` gets #GP; one that touches kernel memory
gets #PF. The interrupt frame's `cs` tells you which ring the fault came
from. From ring 3, it's the program's problem: kill the thread (exit status
`EXIT_KILLED`) and carry on. From ring 0, it's *your* bug: panic.

## Your mission

1. `tss_init` and `tss_set_kernel_stack`; update `esp0` on every thread switch.
2. `thread_create_user`: a thread that runs in ring 3 in a given address
   space. The scheduler must switch CR3 to the thread's address space.
3. `syscall_init`, `user_range_ok`, and system calls `SYS_EXIT`, `SYS_WRITE`
   (fd 1 and 2 → console), `SYS_YIELD`, `SYS_GETPID` (the thread id for now),
   `SYS_SLEEP`. Unknown numbers return `-ENOSYS`.
4. CPU exceptions from ring 3 kill the thread instead of panicking.

## Questions before you code

1. Draw the stack you build for the `iret` into ring 3, word by word. What
   EFLAGS value do you want (think about IF)?
2. After that `iret`, what are `ds`/`es`/`fs`/`gs`? Should they be user
   selectors? What would happen with kernel selectors still loaded?
3. Why does each thread need its *own* kernel stack, and `esp0` updated on
   every switch? What goes wrong if two user threads share one?
4. `int 0x80` goes through an *interrupt gate*, so IF is cleared on entry.
   `SYS_SLEEP` waits for timer ticks. What must the handler do first?
5. Which checks make `user_range_ok(as, 0xBFFFFFF0, 0x100, 0)` fail? And
   `(as, 0x40001000, 0xFFFFFFF0, 0)`?
6. The kernel kills a faulting user thread from inside the exception handler.
   Whose stack is it running on at that moment? Is it safe to call
   `thread_exit` there?

## Milestones

1. A user thread runs a hand-copied infinite loop in ring 3, and the timer still
   preempts it (proof: other threads keep running).
2. `int 0x80` from ring 3 reaches your handler; `SYS_WRITE` prints.
3. A user `cli` kills only that thread.

## The checkpoint verifies

The checkpoints copy small hand-written ring-3 programs (`tests/blobs.asm`)
into fresh address spaces and run them. They check: TSS loaded (task
register = `0x28`, a busy 32-bit TSS); `user_range_ok` against
read-only pages, unmapped pages, kernel addresses, a wrap-around range and
zero-length ranges; `write` from ring 3 reaching the serial port and returning
its length; `-EFAULT`/`-EBADF`/`-ENOSYS` for bad arguments; registers other
than `eax` preserved across a system call; `cli`, reading kernel memory and
jumping into kernel code each killing just that thread; `getpid`/`yield`/`sleep`;
the timer preempting ring-3 code; and no leaks after 20 killed threads.

## Hints

<details><summary>Hint 1: how the user thread starts</summary>

Create it as an ordinary kernel thread whose entry is a small routine that
loads the user data selector into the data segment registers and executes
`iret` on a hand-built frame: user `ss`, user `esp`, EFLAGS with IF set, user
`cs`, entry point. Pass the entry point and user stack through the thread's
initial registers or a small struct.
</details>

<details><summary>Hint 2: the TSS descriptor</summary>

It's a *system* descriptor: S bit = 0, type "32-bit TSS (available)", base
= the address of your TSS struct, limit = its size − 1, byte granularity.
`ss0` = the kernel data selector. Set the I/O map base to the TSS size
(meaning "no I/O permission bitmap").
</details>

<details><summary>Hint 3: the first user interrupt triple-faults</summary>

That's almost always the TSS: `esp0` is 0 or points to a stack that isn't
mapped, or `ss0` is wrong, or `ltr` was never executed. In the QEMU monitor,
`info registers` shows `TR` with its base and limit.
</details>

<details><summary>Hint 4: user_range_ok</summary>

Check the arithmetic first, without overflow: `ptr >= USER_SPACE_START`,
`ptr < USER_SPACE_END`, `len <= USER_SPACE_END - ptr`. Then walk every page
from `ptr` rounded down to `ptr + len`, checking each with `vmm_translate`.
</details>

## Going further

- Use the `sysenter`/`sysexit` instructions instead of `int 0x80`. Measure
  the difference.
- Give user threads an FPU (`fxsave`/`fxrstor` on switch, or lazily via #NM).
- A proper `copy_from_user` that handles page faults during the copy instead
  of pre-checking.

## Reflect

- List every way a ring-3 program could try to crash or take over your
  kernel. For each, which mechanism stops it?
- Why did lesson 05 insist that the common stub reload the kernel data
  segments? What would a user program be able to do otherwise?

## References

- Intel SDM Vol. 3A: §5.5-5.8 (privilege levels, call/interrupt privilege
  checks), §6.12.1 (stack switching on interrupts), Chapter 7 (TSS)
- Intel SDM Vol. 2: `IRET`, `LTR`, `STR`
- OSDev wiki: "Getting to Ring 3", "Task State Segment", "System Calls"
