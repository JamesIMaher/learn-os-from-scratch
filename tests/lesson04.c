/* Lesson 04 checkpoints: the Global Descriptor Table. */
#include "harness.h"
#include "arch/gdt.h"

struct __attribute__((packed)) gdtr { uint16_t limit; uint32_t base; };

static uint16_t sreg(int which)
{
    uint16_t v = 0;
    switch (which) {
    case 0: __asm__ volatile("mov %%cs, %0" : "=r"(v)); break;
    case 1: __asm__ volatile("mov %%ds, %0" : "=r"(v)); break;
    case 2: __asm__ volatile("mov %%es, %0" : "=r"(v)); break;
    case 3: __asm__ volatile("mov %%fs, %0" : "=r"(v)); break;
    case 4: __asm__ volatile("mov %%gs, %0" : "=r"(v)); break;
    case 5: __asm__ volatile("mov %%ss, %0" : "=r"(v)); break;
    }
    return v;
}

/* The CPU itself decodes the descriptor for us: LAR returns its access rights
 * and LSL its limit (in bytes), or fail (ZF=0) if the selector is unusable. */
static bool lar(uint32_t sel, uint32_t *rights)
{
    uint8_t ok;
    __asm__ volatile("lar %2, %0; setz %1" : "=r"(*rights), "=q"(ok) : "r"(sel) : "cc");
    return ok;
}
static bool lsl(uint32_t sel, uint32_t *limit)
{
    uint8_t ok;
    __asm__ volatile("lsl %2, %0; setz %1" : "=r"(*limit), "=q"(ok) : "r"(sel) : "cc");
    return ok;
}

#define AR_PRESENT   (1u << 15)
#define AR_DPL(x)    (((x) >> 13) & 3)
#define AR_SEGMENT   (1u << 12)   /* code/data (not a system descriptor) */
#define AR_CODE      (1u << 11)
#define AR_RW        (1u << 9)    /* readable (code) / writable (data)   */
#define AR_32BIT     (1u << 22)
#define AR_4K_GRAN   (1u << 23)

static void check_segment(uint32_t sel, bool code, int dpl)
{
    uint32_t ar = 0, limit = 0;
    CHECK(lar(sel, &ar));
    CHECK(ar & AR_PRESENT);
    CHECK(ar & AR_SEGMENT);
    CHECK_EQ(AR_DPL(ar), dpl);
    CHECK_EQ(!!(ar & AR_CODE), code);
    CHECK(ar & AR_RW);
    CHECK(ar & AR_32BIT);
    CHECK(ar & AR_4K_GRAN);
    CHECK(lsl(sel, &limit));
    CHECK_EQ(limit, 0xFFFFFFFF);
}

void lesson04(void)
{
    t_lesson(4, "The Global Descriptor Table");
    gdt_init();

    t_begin("the GDT is loaded and has room for 6 descriptors");
    struct gdtr g = { 0, 0 };
    __asm__ volatile("sgdt %0" : "=m"(g));
    CHECK(g.limit >= 6 * 8 - 1);
    CHECK(g.base != 0);
    CHECK((g.limit + 1) % 8 == 0);
    if (g.base) {
        const uint32_t *null_desc = (const uint32_t *)(uintptr_t)g.base;
        CHECK_EQ(null_desc[0], 0);
        CHECK_EQ(null_desc[1], 0);
    }
    t_end();

    t_begin("segment registers use the new kernel selectors");
    CHECK_EQ(sreg(0), GDT_KERNEL_CODE);
    CHECK_EQ(sreg(1), GDT_KERNEL_DATA);
    CHECK_EQ(sreg(2), GDT_KERNEL_DATA);
    CHECK_EQ(sreg(3), GDT_KERNEL_DATA);
    CHECK_EQ(sreg(4), GDT_KERNEL_DATA);
    CHECK_EQ(sreg(5), GDT_KERNEL_DATA);
    t_end();

    t_begin("kernel code segment: flat 4 GiB, ring 0, 32-bit");
    check_segment(GDT_KERNEL_CODE, true, 0);
    t_end();
    t_begin("kernel data segment: flat 4 GiB, ring 0, 32-bit");
    check_segment(GDT_KERNEL_DATA, false, 0);
    t_end();
    t_begin("user code segment: flat 4 GiB, ring 3, 32-bit");
    check_segment(GDT_USER_CODE, true, 3);
    t_end();
    t_begin("user data segment: flat 4 GiB, ring 3, 32-bit");
    check_segment(GDT_USER_DATA, false, 3);
    t_end();

    t_begin("memory still works through the new segments");
    static volatile uint32_t probe;
    probe = 0x600DF00D;
    CHECK_EQ(probe, 0x600DF00D);
    t_end();
}
