/* Lesson 15 checkpoints: a shell, driven by real keystrokes. */
#include "harness.h"
#include "proc/process.h"
#include "proc/thread.h"

void lesson15(void)
{
    t_lesson(15, "A shell");

    t_begin("the shell runs commands typed on the keyboard");
    int pid = process_spawn("/bin/sh");
    CHECK(pid > 0);
    if (pid <= 0) { t_end(); return; }
    thread_sleep_ms(1000);
    t_expect_serial("$ ");

    /* echo collapses runs of spaces between words: */
    t_send_keys("e c h o spc o n e spc spc spc t w o ret");
    t_expect_serial("one two");
    thread_sleep_ms(1000);

    /* backspace edits the line before it's run: */
    t_send_keys("e c h o spc z q 7 x backspace k ret");
    t_expect_serial("zq7k");
    thread_sleep_ms(1000);

    t_send_keys("l s spc slash ret");
    t_expect_serial("hello.txt");
    thread_sleep_ms(1000);

    t_send_keys("l s spc slash d o c s ret");
    t_expect_serial("a.txt");
    thread_sleep_ms(1000);

    t_send_keys("c a t spc slash d o c s slash b dot t x t ret");
    t_expect_serial("And this is b.txt.");
    thread_sleep_ms(1000);

    /* a word that isn't a builtin runs /bin/<word>; a nonzero status is shown: */
    t_send_keys("s h e l l t e s t ret");
    t_expect_serial("shelltest-ran-ok");
    t_expect_serial("[status 3]");
    thread_sleep_ms(1500);

    t_send_keys("f r o b n i c a t e ret");
    t_expect_serial("frobnicate: command not found");
    thread_sleep_ms(1000);

    t_send_keys("c a t spc slash n o p e ret");
    t_expect_serial("cat: /nope: not found");
    thread_sleep_ms(1000);

    t_send_keys("e x i t ret");
    int status = -1;
    CHECK_EQ(process_wait(pid, &status), pid);
    CHECK_EQ(status, 0);
    t_end();
}
