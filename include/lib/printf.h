/*
 * printf.h - formatted output.                                 [Lesson 02]
 *
 * Required conversions:   %c %s %d %i %u %x %X %p %%
 * Required flags:         '-' (left-justify) and '0' (zero-pad)
 * Required:               a decimal field width, e.g. %8s %-5d %08x
 * Accepted and ignored:   the 'l' length modifier (long == int on i386)
 *
 * %p prints "0x" followed by exactly 8 lowercase hex digits.
 * %s with a NULL pointer prints "(null)".
 *
 * Not required (but nice for later): precision (%.3s), %lld, %llu.
 */
#pragma once
#include <stdarg.h>
#include <stddef.h>

/*
 * Format into buf, writing at most `size` bytes INCLUDING the terminating
 * NUL. Always NUL-terminates when size > 0. Returns the number of characters
 * that WOULD have been written if buf were large enough (excluding the NUL),
 * exactly like C99 vsnprintf. Why is that return value useful?
 */
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int ksnprintf(char *buf, size_t size, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* Format and send to the console (see kernel/console.h). Returns chars written. */
int kprintf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
