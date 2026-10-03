/* uprintf.c - formatted output for user programs.               [Lesson 13] */
#include "ulib.h"

/* TODO(lesson 13) */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    (void)buf; (void)size; (void)fmt; (void)args;
    return 0;
}
int snprintf(char *buf, size_t size, const char *fmt, ...) { (void)buf; (void)size; (void)fmt; return 0; }
int printf(const char *fmt, ...) { (void)fmt; return 0; }
int puts(const char *s) { (void)s; return 0; }
