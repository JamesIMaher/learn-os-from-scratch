/*
 * panic.h - stop the machine with a message.                   [Lesson 05]
 *
 * Print "KERNEL PANIC: " and the formatted message to the console, then
 * halt forever with interrupts disabled. Must never return.
 */
#pragma once

__attribute__((noreturn, format(printf, 1, 2)))
void panic(const char *fmt, ...);

/* Panic with file and line if `cond` is false. */
#define kassert(cond) \
    do { if (!(cond)) panic("assertion failed: %s (%s:%d)", #cond, __FILE__, __LINE__); } while (0)
