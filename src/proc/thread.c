/* thread.c - see include/proc/thread.h.                 [Lessons 11 and 12] */
#include "proc/thread.h"

/*
 * TODO(lesson 11). The heart of it is a context switch: save the registers
 * the C calling convention says a callee must preserve, save the stack
 * pointer into the old thread, load the new thread's stack pointer, restore
 * its registers, return. "Return" to where? That's the trick to understand.
 * (See switch.asm.) Then: how do you build a NEW thread's stack so that the
 * first switch into it "returns" into entry(arg)?
 *
 * TODO(lesson 12). A user thread starts life as a kernel thread whose first
 * job is to drop into ring 3. The only way down is to fake the stack an
 * interrupt from ring 3 would have left, and IRET.
 */

struct thread {
    int todo_replace_me;   /* TODO(lesson 11) */
};

void sched_init(void) { }
void sched_tick(void) { }
thread_t *thread_create(const char *name, void (*entry)(void *arg), void *arg)
{
    (void)name; (void)entry; (void)arg;
    return 0;
}
void thread_yield(void) { }
void thread_exit(int status) { (void)status; for (;;) { } }
int thread_join(thread_t *t) { (void)t; return 0; }
void thread_sleep_ms(uint32_t ms) { (void)ms; }
thread_t *thread_current(void) { return 0; }
int thread_id(const thread_t *t) { (void)t; return 0; }
const char *thread_name(const thread_t *t) { (void)t; return "?"; }

thread_t *thread_create_user(const char *name, address_space_t *as,
                             uintptr_t entry, uintptr_t user_stack_top)
{
    (void)name; (void)as; (void)entry; (void)user_stack_top;
    return 0;
}
