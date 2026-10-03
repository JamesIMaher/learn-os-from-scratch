/* panic.c - see include/kernel/panic.h.                         [Lesson 05] */
#include "kernel/panic.h"

void panic(const char *fmt, ...)
{
    (void)fmt;
    /* TODO(lesson 05): print the message (stdarg.h works fine in a kernel:
     * why?), then stop the CPU for good. */
    for (;;) { }
}
