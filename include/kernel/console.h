/*
 * console.h - where kernel output goes.               [Lessons 02 and 03]
 *
 * kprintf() calls console_putc(). In lesson 02 the console is just the
 * serial port. In lesson 03 it should ALSO draw on the VGA screen.
 */
#pragma once

void console_putc(char c);
void console_write(const char *s);
