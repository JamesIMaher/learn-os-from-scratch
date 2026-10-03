# Lesson 11: Threads and scheduling

> **Goal:** many threads of execution sharing one CPU: created, switched,
> preempted by the timer, sleeping, joined, and synchronized with mutexes.
> **Files:** `src/proc/thread.c`, `src/proc/switch.asm`, `src/proc/mutex.c`,
> `include/proc/mutex.h` (fill in the struct), timer IRQ → `sched_tick`
> **Checkpoint:** `make check L=11`

## Why this matters

This is where your kernel becomes a *multitasking* kernel. The core idea is
almost magical: a thread is nothing more than a saved stack pointer. Switch
stacks and you've switched threads, because everything else (the registers,
the return addresses, the whole call chain) lives on the stack.

## Background

### The context switch

A function `context_switch(&old->saved_esp, new->saved_esp)`:
push the registers the calling convention says a callee must preserve, store
`esp` into the old thread, load the new thread's `esp`, pop *its* saved
registers, and `ret`. That `ret` returns into wherever the *new* thread was
when it last called `context_switch`. From each thread's point of view,
`context_switch` is just a function call that takes a while.

### Starting a new thread

A brand-new thread has never called `context_switch`. So you **fake it**: build
its stack so that it looks as if it had, with a "return address" that leads
to code which calls `entry(arg)` and then `thread_exit`.

### The scheduler

Thread states: running, ready (waiting for the CPU), blocked (waiting for
something else: a join, a mutex, a sleep timer), finished. A **ready queue**
holds runnable threads in FIFO order (round robin). When the current thread
yields, blocks or uses up its **time slice**, pick the next ready thread and
switch to it. When nobody is ready, something must still run: an **idle
thread** that halts until the next interrupt.

### Preemption

`sched_tick()` is called from the timer IRQ. When the current thread's slice
is used up, it switches threads *from inside the interrupt handler*. That's
fine: the interrupted thread's state is on its own stack and it will finish
the interrupt (and `iret`) whenever it's next scheduled. Think about what
happens to the PIC's EOI if you switch away before sending it.

### Races, on one CPU

With preemption, a thread can be interrupted between any two instructions.
Kernel data structures (the ready queue!) must not be seen half-updated. On
a single CPU the tool is: disable interrupts around the critical section, and
restore the previous interrupt state afterwards. A **mutex** gives threads
the same guarantee for longer sections, but blocks waiters instead of
disabling interrupts.

## Your mission

Implement `include/proc/thread.h` (except `thread_create_user`, lesson 12)
and `include/proc/mutex.h`. Call `sched_tick` from your timer handler.
Then in `kmain`, start two threads that print "A" and "B" forever and watch
the timer interleave them.

## Questions before you code

1. Which registers does cdecl require a function to preserve? Why does your
   switch only need to save those?
2. Draw the initial stack of a new thread, word by word, so that the first
   `context_switch` into it ends up calling `entry(arg)`.
3. The new thread starts running in the middle of `context_switch`, called
   from somewhere with interrupts disabled. What does it need to do about the
   interrupt flag?
4. A thread calls `thread_exit`. Its stack is still in use (it's running
   on it!). Who frees it, and when is that safe?
5. `thread_join(t)` when `t` has already exited vs. when it hasn't: what does
   each case do?
6. `mutex_lock`: between "it's free" and "it's mine", what could the timer do?
7. A sleeping thread: who wakes it up, and how do they know when?

## Milestones

1. Cooperative only: two threads ping-pong with `thread_yield`.
2. `thread_exit` and `thread_join` with exit status.
3. Preemption: two threads that never yield both make progress.
4. `thread_sleep_ms`, and an idle thread so everyone can sleep at once.
5. Mutexes.

## The checkpoint verifies

`sched_init` adopts the running code as a thread; create/arguments/exit
status/join; names and ids; strict round-robin order with `thread_yield`;
preemption of threads that never yield; sleep duration and that others run
meanwhile; a mutex protecting a read-*yield*-write sequence across 4 threads;
100 threads with unique ids and correct statuses, without leaking stacks.

## Hints

<details><summary>Hint 1: where the trampoline gets entry and arg</summary>

When your fake stack is "popped" by the second half of `context_switch`, the
callee-saved registers get whatever values you put there. Put `entry` and
`arg` in two of those slots and make the fake return address point to a small
assembly trampoline that enables interrupts, pushes `arg`, calls `entry`, and
then calls `thread_exit(0)`.
</details>

<details><summary>Hint 2: everything freezes after the first preemption</summary>

If your dispatcher sends the EOI *after* the handler returns, and the
handler switched to another thread, the EOI only happens when the
original thread resumes. Until then, no more timer interrupts. Send the EOI
before calling IRQ handlers.
</details>

<details><summary>Hint 3: blocking without losing wakeups</summary>

The pattern is: disable interrupts; check the condition; if you must wait,
mark yourself blocked, put yourself on the right wait list, and call the
scheduler, all *still with interrupts disabled*; re-check the condition when
you wake (`while`, not `if`); restore interrupts. The scheduler switches to
another thread, which will have its own interrupt state.
</details>

<details><summary>Hint 4: the thread that runs before sched_init</summary>

`sched_init` doesn't create a stack for the current code: it already has one
(your boot stack). It only allocates a `thread_t` describing it, marked
running.
</details>

## Debugging when stuck

- gdb: `info threads` won't help (QEMU shows one CPU), but you can print your
  ready queue: `p *ready_head`, `p *ready_head->next`...
- A bug in the initial stack layout usually crashes on the first switch into a
  new thread. Break on `context_switch` and `stepi` through the `ret`.

## Going further

- Priorities. What's "priority inversion" and how can a mutex cause it?
- Semaphores and condition variables built on the same blocking primitive.
- A `ps`-style dump of all threads on a key press.

## Reflect

- What *exactly* is a thread, in your implementation? List every piece of
  state that makes one thread different from another.
- On a multi-core machine, why would "disable interrupts" no longer be
  enough to protect the ready queue?

## References

- *Operating Systems: Three Easy Pieces* (free online): chapters on processes,
  scheduling, locks, condition variables
- System V ABI i386: callee-saved registers
- OSDev wiki: "Context Switching", "Scheduling Algorithms"
