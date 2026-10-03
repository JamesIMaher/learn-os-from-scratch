/* printf.c - see include/lib/printf.h.                          [Lesson 02] */
#include "lib/printf.h"

/*
 * TODO(lesson 02). Suggested order: get kvsnprintf right first (it's pure
 * logic, so the tests can check it thoroughly), then build ksnprintf and
 * kprintf on top of it. How big a buffer does kprintf need? Do you even need
 * one?
 */

int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    (void)buf; (void)size; (void)fmt; (void)args;
    return 0;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...)
{
    (void)buf; (void)size; (void)fmt;
    return 0;
}

int kprintf(const char *fmt, ...)
{
    (void)fmt;
    return 0;
}
