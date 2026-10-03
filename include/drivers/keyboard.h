/*
 * keyboard.h - PS/2 keyboard driver.                           [Lesson 07]
 *
 * The keyboard controller raises IRQ 1 and hands you SCANCODES (scancode set
 * 1), not characters. Translating them is your job: key presses vs releases,
 * the shift keys, and a buffer so no keystroke is lost if nobody is reading.
 *
 * Characters produced: printable ASCII for the US layout (letters, digits,
 * punctuation, with and without shift), '\n' for Enter, '\b' for Backspace,
 * '\t' for Tab, ' ' for Space. Other keys (arrows, F-keys, ...) produce nothing.
 *
 * The buffer must hold at least 64 characters.
 */
#pragma once

/* Install the IRQ 1 handler. Interrupts must be enabled separately. */
void keyboard_init(void);

/* Return the next buffered character, or -1 immediately if there is none. */
int keyboard_getchar(void);

/* Wait until a character is available and return it. */
char keyboard_read(void);
