/* Lesson 06 checkpoints: the PIC, hardware interrupts, and the timer. */
#include "harness.h"
#include "arch/pic.h"
#include "arch/idt.h"
#include "drivers/timer.h"

static volatile int irq0_hits;
static volatile uint32_t irq0_vector;
static void h_irq0(struct interrupt_frame *f) { irq0_vector = f->vector; irq0_hits++; }

/* CMOS real-time clock: seconds register, waiting out update cycles. */
static uint8_t rtc_seconds(void)
{
    for (;;) {
        t_outb(0x70, 0x0A);
        if (t_inb(0x71) & 0x80) continue;      /* update in progress */
        t_outb(0x70, 0x00);
        return t_inb(0x71);
    }
}

static bool ticks_advance(uint64_t by, uint32_t max_spins)
{
    uint64_t start = timer_ticks();
    for (uint32_t i = 0; i < max_spins; i++) {
        if (timer_ticks() - start >= by) return true;
        __asm__ volatile("pause");
    }
    return false;
}

void lesson06(void)
{
    t_lesson(6, "Hardware interrupts and the timer");

    t_begin("pic_init masks every IRQ except the cascade");
    pic_init();
    CHECK_EQ(t_inb(0x21), 0xFB);
    CHECK_EQ(t_inb(0xA1), 0xFF);
    t_end();

    t_begin("pic_mask / pic_unmask change exactly one line");
    pic_unmask(4);
    CHECK_EQ(t_inb(0x21), 0xEB);
    pic_mask(4);
    CHECK_EQ(t_inb(0x21), 0xFB);
    pic_unmask(12);
    CHECK_EQ(t_inb(0xA1), 0xEF);
    pic_mask(12);
    CHECK_EQ(t_inb(0xA1), 0xFF);
    t_end();

    t_begin("timer_init programs the PIT and unmasks IRQ 0");
    timer_init(100);
    CHECK_EQ(timer_hz(), 100);
    CHECK_EQ(t_inb(0x21) & 1, 0);
    CHECK((t_eflags() & (1u << 9)) == 0);   /* timer_init must not sti */
    t_end();

    __asm__ volatile("sti");

    t_begin("ticks keep arriving (so the EOI is being sent)");
    CHECK(ticks_advance(10, 400000000u));
    t_end();

    t_begin("IRQ 0 arrives on vector 32");
    isr_handler_t old = isr_register(PIC_IRQ_BASE + 0, h_irq0);
    irq0_hits = 0;
    for (uint32_t i = 0; i < 400000000u && irq0_hits < 5; i++) __asm__ volatile("pause");
    CHECK(irq0_hits >= 5);
    CHECK_EQ(irq0_vector, 32);
    isr_register(PIC_IRQ_BASE + 0, old);
    t_end();

    t_begin("the timer runs at 100 Hz (measured against the RTC clock)");
    uint8_t s0 = rtc_seconds();
    while (rtc_seconds() == s0) __asm__ volatile("pause");
    uint64_t t1 = timer_ticks();
    uint8_t s1 = rtc_seconds();
    while (rtc_seconds() == s1) __asm__ volatile("pause");
    uint64_t t2 = timer_ticks();
    uint32_t per_second = (uint32_t)(t2 - t1);
    CHECK(per_second >= 85 && per_second <= 115);
    if (per_second < 85 || per_second > 115) {
        t_puts("@@NOTE measured ticks per second: "); t_putdec((int32_t)per_second); t_putc('\n');
    }
    t_end();

    t_begin("timer_sleep_ms sleeps about the right time");
    uint64_t a = timer_ticks();
    timer_sleep_ms(200);
    uint64_t b = timer_ticks();
    CHECK(b - a >= 20);
    CHECK(b - a <= 25);
    a = timer_ticks();
    timer_sleep_ms(0);
    CHECK(timer_ticks() - a <= 1);
    t_end();

    t_begin("ticks are monotonic");
    uint64_t prev = timer_ticks();
    bool mono = true;
    for (int i = 0; i < 200000; i++) {
        uint64_t now = timer_ticks();
        if (now < prev) mono = false;
        prev = now;
    }
    CHECK(mono);
    t_end();
}
