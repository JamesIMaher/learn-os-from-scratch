/* Lesson 05 checkpoints: the IDT, exceptions, and the interrupt frame. */
#include "harness.h"
#include "arch/idt.h"
#include "arch/gdt.h"

struct __attribute__((packed)) idtr { uint16_t limit; uint32_t base; };

static struct interrupt_frame seen;
static volatile int hits;

static void copy_frame(struct interrupt_frame *f)
{
    const uint8_t *s = (const uint8_t *)f;
    uint8_t *d = (uint8_t *)&seen;
    for (size_t i = 0; i < sizeof seen; i++) d[i] = s[i];
    hits++;
}

static void h_breakpoint(struct interrupt_frame *f) { copy_frame(f); }

static void h_syscall(struct interrupt_frame *f)
{
    copy_frame(f);
    f->eax = 0x12345678;          /* changes must survive the return */
    f->ebx = 0x87654321;
}

/* Both faulting instructions below are exactly 2 bytes long, so the
 * handler "fixes" the fault by skipping them. */
static void h_skip2(struct interrupt_frame *f) { copy_frame(f); f->eip += 2; }

static void h_other(struct interrupt_frame *f) { (void)f; }

void lesson05(void)
{
    t_lesson(5, "Interrupts and exceptions");
    idt_init();

    t_begin("the IDT is loaded and covers vector 0x80");
    struct idtr r = { 0, 0 };
    __asm__ volatile("sidt %0" : "=m"(r));
    CHECK(r.base != 0);
    CHECK(r.limit >= 0x80 * 8 + 7);
    CHECK((t_eflags() & (1u << 9)) == 0);   /* idt_init must not sti */
    t_end();

    t_begin("isr_register returns the previous handler");
    isr_handler_t old = isr_register(3, h_breakpoint);
    CHECK(isr_register(3, h_other) == h_breakpoint);
    CHECK(isr_register(3, h_breakpoint) == h_other);
    t_end();

    t_begin("int3 reaches the handler with a correct frame, then resumes");
    uint32_t after = 0;
    hits = 0;
    __asm__ volatile("int3\n1: movl $1b, %0" : "=r"(after));
    CHECK_EQ(hits, 1);
    CHECK_EQ(seen.vector, 3);
    CHECK_EQ(seen.err_code, 0);
    CHECK_EQ(seen.eip, after);
    CHECK_EQ(seen.cs, GDT_KERNEL_CODE);
    CHECK_EQ(seen.ds, GDT_KERNEL_DATA);
    CHECK_EQ(seen.es, GDT_KERNEL_DATA);
    CHECK(seen.eflags & 2);                  /* bit 1 of EFLAGS is always 1 */
    isr_register(3, old);
    t_end();

    t_begin("general registers are saved in the frame, and changes are restored");
    old = isr_register(0x80, h_syscall);
    uint32_t a = 0xA1, b = 0xB2, c = 0xC3, d = 0xD4, si = 0xE5, di = 0xF6;
    hits = 0;
    __asm__ volatile("int $0x80"
                     : "+a"(a), "+b"(b), "+c"(c), "+d"(d), "+S"(si), "+D"(di)
                     :: "memory", "cc");
    CHECK_EQ(hits, 1);
    CHECK_EQ(seen.vector, 0x80);
    CHECK_EQ(seen.eax, 0xA1); CHECK_EQ(seen.ebx, 0xB2); CHECK_EQ(seen.ecx, 0xC3);
    CHECK_EQ(seen.edx, 0xD4); CHECK_EQ(seen.esi, 0xE5); CHECK_EQ(seen.edi, 0xF6);
    CHECK_EQ(a, 0x12345678);
    CHECK_EQ(b, 0x87654321);
    CHECK_EQ(c, 0xC3); CHECK_EQ(d, 0xD4); CHECK_EQ(si, 0xE5); CHECK_EQ(di, 0xF6);
    isr_register(0x80, old);
    t_end();

    t_begin("divide error (#DE) is delivered on vector 0");
    old = isr_register(0, h_skip2);
    hits = 0;
    __asm__ volatile("movl $1, %%eax\n xorl %%edx, %%edx\n xorl %%ecx, %%ecx\n"
                     ".byte 0xF7, 0xF1   # div %%ecx (2 bytes)\n"
                     ::: "eax", "ecx", "edx", "cc");
    CHECK_EQ(hits, 1);
    CHECK_EQ(seen.vector, 0);
    CHECK_EQ(seen.err_code, 0);
    isr_register(0, old);
    t_end();

    t_begin("general protection fault (#GP) delivers its error code");
    old = isr_register(13, h_skip2);
    hits = 0;
    uint16_t ds_after = 0;
    __asm__ volatile("movw $0x0FF0, %%ax\n"
                     ".byte 0x8E, 0xD8   # mov %%ax, %%ds (2 bytes)\n"
                     "movw %%ds, %0"
                     : "=r"(ds_after) :: "eax", "memory");
    CHECK_EQ(hits, 1);
    CHECK_EQ(seen.vector, 13);
    CHECK_EQ(seen.err_code, 0x0FF0);   /* the offending selector */
    CHECK_EQ(ds_after, GDT_KERNEL_DATA);
    isr_register(13, old);
    t_end();

    t_begin("handlers can be invoked repeatedly");
    old = isr_register(3, h_breakpoint);
    hits = 0;
    for (int i = 0; i < 1000; i++) __asm__ volatile("int3");
    CHECK_EQ(hits, 1000);
    isr_register(3, old);
    t_end();
}
