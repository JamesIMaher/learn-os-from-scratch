/* console.c - see include/kernel/console.h.            [Lessons 02 and 03] */
#include "kernel/console.h"

void console_putc(char c)
{
    (void)c;
    /* TODO(lesson 02): send to the serial port.
     * TODO(lesson 03): ...and to the VGA screen. */
}

void console_write(const char *s)
{
    (void)s;
    /* TODO(lesson 02) */
}
