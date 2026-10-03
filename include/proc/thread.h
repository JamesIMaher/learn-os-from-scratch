/*
 * thread.h - kernel threads and the scheduler.          [Lessons 11 and 12]
 *
 * A thread is a CPU context (registers + its own kernel stack) that the
 * scheduler can stop and resume. Scheduling is round-robin and PREEMPTIVE:
 * the timer interrupt forces a switch when a thread's time slice runs out,
 * so a thread that never yields still can't hog the CPU.
 */
#pragma once
#include <stdint.h>
#include "mm/vmm.h"

typedef struct thread thread_t;    /* you define the struct */

/* Exit status of a thread killed because it faulted in user mode (lesson 12). */
#define EXIT_KILLED (-128)

/* Turn the code that is already running (kmain) into the first thread and
 * start the scheduler. Call after heap_init and timer_init. */
void      sched_init(void);

/* Called by the timer interrupt on every tick. Decides when to preempt. */
void      sched_tick(void);

/* Create a runnable kernel thread that calls entry(arg). Returning from entry
 * is the same as thread_exit(0). NULL if out of memory. */
thread_t *thread_create(const char *name, void (*entry)(void *arg), void *arg);

/* Give up the CPU to the next runnable thread (if there is one). */
void      thread_yield(void);

/* Finish the current thread with `status`. */
__attribute__((noreturn)) void thread_exit(int status);

/* Block until `t` has exited, free it, and return its exit status. Each
 * thread is joined exactly once. (Can a thread free its own stack?) */
int       thread_join(thread_t *t);

/* Block for at least `ms` milliseconds, letting other threads run meanwhile. */
void      thread_sleep_ms(uint32_t ms);

thread_t   *thread_current(void);
int         thread_id(const thread_t *t);    /* unique, positive */
const char *thread_name(const thread_t *t);

/* ---- Lesson 12 ---------------------------------------------------------- */

/*
 * Create a thread that runs in RING 3 inside address space `as`, starting at
 * `entry` with its stack pointer at `user_stack_top`. The caller has already
 * mapped the code and stack with VMM_USER. The thread switches to `as`
 * whenever it runs. A CPU exception raised in user mode kills only that
 * thread (exit status EXIT_KILLED); the kernel keeps running.
 * The caller still owns `as` and destroys it after joining.
 */
thread_t *thread_create_user(const char *name, address_space_t *as,
                             uintptr_t entry, uintptr_t user_stack_top);
