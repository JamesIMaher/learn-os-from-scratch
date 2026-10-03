/* keyboard.c - see include/drivers/keyboard.h.                  [Lesson 07] */
#include "drivers/keyboard.h"
#include "arch/io.h"
#include "arch/pic.h"

/*
 * TODO(lesson 07). Things to work out:
 *   - which port the scancode is read from, and why you MUST read it,
 *   - how a release scancode differs from a press scancode,
 *   - a scancode -> ASCII table (normal and shifted) for set 1,
 *   - a buffer between the interrupt handler (producer) and readers
 *     (consumers). What happens when it's full?
 */

void keyboard_init(void) { }
int keyboard_getchar(void) { return -1; }
char keyboard_read(void) { return 0; }
