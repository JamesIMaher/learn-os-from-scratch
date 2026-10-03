/* Lesson 07 checkpoints: the PS/2 keyboard. */
#include "harness.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"

/* Collect up to n characters, giving up after `ms` milliseconds. */
static int collect(char *out, int n, uint32_t ms)
{
    uint64_t end = timer_ticks() + (uint64_t)ms * timer_hz() / 1000;
    int got = 0;
    while (got < n && timer_ticks() < end) {
        int c = keyboard_getchar();
        if (c >= 0) out[got++] = (char)c;
        else __asm__ volatile("hlt");
    }
    out[got] = 0;
    return got;
}

void lesson07(void)
{
    t_lesson(7, "The keyboard");
    char got[64];

    t_begin("keyboard_init; nothing buffered yet");
    keyboard_init();
    timer_sleep_ms(50);
    while (keyboard_getchar() >= 0) { }
    CHECK_EQ(keyboard_getchar(), -1);
    t_end();

    t_begin("letters, shift, digits, punctuation, enter, backspace, tab");
    t_send_keys("h i shift-h shift-1 1 spc minus shift-minus ret backspace tab "
                "shift-a z slash shift-slash");
    collect(got, 15, 6000);
    CHECK_STR(got, "hiH!1 -_\n\b\tAz/?");
    CHECK_EQ(keyboard_getchar(), -1);    /* key releases produce nothing */
    t_end();

    t_begin("keystrokes are buffered when nobody is reading");
    t_send_keys("a b c d e f g h i j k l m n o p q r s t");
    timer_sleep_ms(2500);                 /* don't read while they arrive */
    collect(got, 20, 3000);
    CHECK_STR(got, "abcdefghijklmnopqrst");
    t_end();

    t_begin("keyboard_read waits for a key");
    t_send_keys("x");
    char c = keyboard_read();
    CHECK_EQ(c, 'x');
    t_end();
}
