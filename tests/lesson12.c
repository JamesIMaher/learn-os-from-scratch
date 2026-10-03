/* Lesson 12 checkpoints: user mode, the TSS, and system calls. */
#include "harness.h"
#include "arch/gdt.h"
#include "proc/thread.h"
#include "proc/syscall.h"
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "drivers/timer.h"
#include "abi/errno.h"

#define CODE_PAGE  0x40000000u
#define DATA_PAGE  0x40001000u
#define STACK_TOP  0xC0000000u

#define DECLARE(name) extern const uint8_t blob_##name[], blob_##name##_end[]
DECLARE(hello); DECLARE(badargs); DECLARE(cli); DECLARE(peek); DECLARE(jump);
DECLARE(pid); DECLARE(spin); DECLARE(regs);

struct user_run {
    address_space_t *as;
    thread_t *t;
    uintptr_t data_frame;
};

static bool start_blob(struct user_run *r, const uint8_t *start, const uint8_t *end)
{
    r->as = vmm_create_space();
    r->t = 0;
    if (!r->as) return false;
    uintptr_t code = pmm_alloc_frame(), stack = pmm_alloc_frame();
    r->data_frame = pmm_alloc_frame();
    uint8_t *dst = (uint8_t *)code;              /* identity mapped */
    for (const uint8_t *p = start; p < end; p++) *dst++ = *p;
    for (int i = 0; i < 1024; i++) ((uint32_t *)r->data_frame)[i] = 0;
    vmm_map(r->as, CODE_PAGE, code, VMM_USER);
    vmm_map(r->as, DATA_PAGE, r->data_frame, VMM_USER | VMM_WRITE);
    vmm_map(r->as, STACK_TOP - PAGE_SIZE, stack, VMM_USER | VMM_WRITE);
    r->t = thread_create_user("blob", r->as, CODE_PAGE, STACK_TOP);
    return r->t != 0;
}

static int finish_blob(struct user_run *r)
{
    int status = r->t ? thread_join(r->t) : -999;
    if (r->as) vmm_destroy_space(r->as);
    return status;
}

static int run_blob(const uint8_t *start, const uint8_t *end)
{
    struct user_run r;
    if (!start_blob(&r, start, end)) { CHECK(!"could not start user thread"); }
    return finish_blob(&r);
}

#define RUN(name) run_blob(blob_##name, blob_##name##_end)

void lesson12(void)
{
    t_lesson(12, "User mode and system calls");

    t_begin("the TSS is installed and loaded");
    tss_init();
    uint16_t tr = 0;
    __asm__ volatile("str %0" : "=r"(tr));
    CHECK_EQ(tr, GDT_TSS);
    uint32_t ar = 0;
    uint8_t ok;
    __asm__ volatile("lar %2, %0; setz %1" : "=r"(ar), "=q"(ok) : "r"((uint32_t)GDT_TSS) : "cc");
    CHECK(ok);
    CHECK_EQ((ar >> 8) & 0x9F, 0x8B);   /* present, system, busy 32-bit TSS */
    t_end();

    syscall_init();

    t_begin("user_range_ok accepts user memory and rejects everything else");
    address_space_t *as = vmm_create_space();
    uintptr_t f = pmm_alloc_frame();
    vmm_map(as, DATA_PAGE, f, VMM_USER);         /* read-only for the user */
    CHECK(user_range_ok(as, DATA_PAGE, 16, 0));
    CHECK(user_range_ok(as, DATA_PAGE, PAGE_SIZE, 0));
    CHECK(!user_range_ok(as, DATA_PAGE, 16, 1));     /* not writable */
    CHECK(!user_range_ok(as, DATA_PAGE, PAGE_SIZE + 1, 0));
    CHECK(!user_range_ok(as, DATA_PAGE + PAGE_SIZE, 1, 0));
    CHECK(!user_range_ok(as, 0x00100000, 4, 0));     /* kernel memory */
    CHECK(!user_range_ok(as, DATA_PAGE, 0xFFFFFFFFu, 0));
    CHECK(!user_range_ok(as, 0xFFFFF000u, 0x2000, 0));
    CHECK(user_range_ok(as, 0, 0, 1));           /* nothing to touch */
    CHECK(!user_range_ok(vmm_kernel_space(), DATA_PAGE, 16, 0));
    vmm_destroy_space(as);
    t_end();

    t_begin("a ring-3 program can call write() and exit()");
    CHECK_EQ(RUN(hello), 16);
    t_expect_serial("ring3-hello-9c1");
    t_end();

    t_begin("system calls validate their arguments");
    CHECK_EQ(RUN(badargs), 0x55);
    t_end();

    t_begin("system calls preserve the caller's registers");
    CHECK_EQ(RUN(regs), 0x42);
    t_end();

    t_begin("a privileged instruction kills only the offending thread");
    CHECK_EQ(RUN(cli), EXIT_KILLED);
    t_end();

    t_begin("user code can't read kernel memory");
    CHECK_EQ(RUN(peek), EXIT_KILLED);
    t_end();

    t_begin("user code can't execute kernel code");
    CHECK_EQ(RUN(jump), EXIT_KILLED);
    t_end();

    t_begin("getpid, yield, sleep from ring 3");
    struct user_run r;
    uint64_t t0 = timer_ticks();
    if (start_blob(&r, blob_pid, blob_pid_end)) {
        int id = thread_id(r.t);
        CHECK_EQ(finish_blob(&r), id);
        CHECK(timer_ticks() - t0 >= 5);
    } else {
        CHECK(!"could not start user thread");
    }
    t_end();

    t_begin("the timer preempts ring-3 code too");
    if (start_blob(&r, blob_spin, blob_spin_end)) {
        volatile uint32_t *data = (volatile uint32_t *)r.data_frame;
        thread_sleep_ms(100);
        uint32_t c1 = data[0];
        thread_sleep_ms(100);
        uint32_t c2 = data[0];
        CHECK(c1 > 0);
        CHECK(c2 > c1);
        data[1] = 1;                              /* tell it to stop */
        CHECK_EQ(finish_blob(&r), 0x77);
    } else {
        CHECK(!"could not start user thread");
    }
    t_end();

    t_begin("the kernel is fine after all that");
    struct t_mem m0;
    t_mem_snapshot(&m0);
    for (int i = 0; i < 20; i++) RUN(cli);
    CHECK(t_mem_no_leak(&m0, 16384));
    CHECK_EQ(RUN(hello), 16);
    t_end();
}
