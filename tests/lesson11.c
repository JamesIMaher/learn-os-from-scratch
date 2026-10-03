/* Lesson 11 checkpoints: threads, scheduling, sleeping, mutexes. */
#include "harness.h"
#include "proc/thread.h"
#include "proc/mutex.h"
#include "mm/heap.h"
#include "mm/pmm.h"
#include "drivers/timer.h"

static char order[32];
static volatile int order_len;

static void pingpong(void *arg)
{
    char me = (char)(uintptr_t)arg;
    for (int i = 0; i < 3; i++) {
        order[order_len++] = me;
        thread_yield();
    }
}

static volatile void *seen_arg;
static void exit7(void *arg) { seen_arg = arg; thread_exit(7); }
static void returns(void *arg) { (void)arg; }

static volatile int stop;
static volatile uint32_t spin_count[2];
static void spinner(void *arg)
{
    int i = (int)(uintptr_t)arg;
    while (!stop) spin_count[i]++;     /* never yields */
}

static mutex_t lock;
static volatile int shared;
static void incrementer(void *arg)
{
    (void)arg;
    for (int i = 0; i < 500; i++) {
        mutex_lock(&lock);
        int tmp = shared;
        thread_yield();                 /* invite trouble inside the lock */
        shared = tmp + 1;
        mutex_unlock(&lock);
    }
}

static void exit_index(void *arg) { thread_exit((int)(uintptr_t)arg); }

static volatile int woke;
static void sleeper(void *arg) { (void)arg; thread_sleep_ms(100); woke = 1; }

void lesson11(void)
{
    t_lesson(11, "Threads and scheduling");

    t_begin("sched_init turns the running code into a thread");
    sched_init();
    thread_t *me = thread_current();
    CHECK(me != 0);
    if (!me) { t_end(); return; }
    CHECK(thread_id(me) > 0);
    t_end();

    t_begin("threads run, take arguments, and report exit status");
    thread_t *t = thread_create("exit7", exit7, (void *)0xFEED);
    CHECK(t != 0);
    if (t) {
        CHECK_STR(thread_name(t), "exit7");
        CHECK(thread_id(t) != thread_id(me));
        CHECK_EQ(thread_join(t), 7);
        CHECK(seen_arg == (void *)0xFEED);
    }
    t = thread_create("returns", returns, 0);
    if (t) CHECK_EQ(thread_join(t), 0);
    CHECK(thread_current() == me);
    t_end();

    t_begin("thread_yield switches round-robin");
    order_len = 0;
    thread_t *A = thread_create("A", pingpong, (void *)'A');
    thread_t *B = thread_create("B", pingpong, (void *)'B');
    if (A && B) {
        thread_join(A);
        thread_join(B);
    }
    order[order_len] = 0;
    CHECK_STR(order, "ABABAB");
    t_end();

    t_begin("the timer preempts threads that never yield");
    stop = 0;
    spin_count[0] = spin_count[1] = 0;
    thread_t *s0 = thread_create("spin0", spinner, (void *)0);
    thread_t *s1 = thread_create("spin1", spinner, (void *)1);
    timer_sleep_ms(300);               /* if we ever get here, preemption works */
    stop = 1;
    if (s0) thread_join(s0);
    if (s1) thread_join(s1);
    CHECK(spin_count[0] > 0);
    CHECK(spin_count[1] > 0);
    t_end();

    t_begin("thread_sleep_ms sleeps and lets others run");
    uint64_t a = timer_ticks();
    woke = 0;
    thread_t *z = thread_create("sleeper", sleeper, 0);
    thread_sleep_ms(50);
    CHECK(!woke);
    if (z) thread_join(z);
    CHECK(woke);
    uint64_t b = timer_ticks();
    CHECK(b - a >= 10);
    CHECK(b - a <= 16);
    t_end();

    t_begin("a mutex keeps a read-yield-write sequence safe");
    mutex_init(&lock);
    shared = 0;
    thread_t *w[4];
    for (int i = 0; i < 4; i++) w[i] = thread_create("inc", incrementer, 0);
    for (int i = 0; i < 4; i++) if (w[i]) thread_join(w[i]);
    CHECK_EQ(shared, 2000);
    t_end();

    t_begin("100 threads: unique ids, right statuses, no leaks");
    struct t_mem m0;
    t_mem_snapshot(&m0);
    static thread_t *many[100];
    bool unique = true, status_ok = true;
    for (int i = 0; i < 100; i++) many[i] = thread_create("many", exit_index, (void *)(uintptr_t)i);
    for (int i = 0; i < 100; i++)
        for (int j = 0; j < i; j++)
            if (many[i] && many[j] && thread_id(many[i]) == thread_id(many[j])) unique = false;
    for (int i = 0; i < 100; i++) {
        CHECK(many[i] != 0);
        if (many[i] && thread_join(many[i]) != i) status_ok = false;
    }
    CHECK(unique);
    CHECK(status_ok);
    CHECK(t_mem_no_leak(&m0, 16384));
    t_end();
}
