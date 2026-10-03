/* vga.c - see include/drivers/vga.h.                            [Lesson 03] */
#include "drivers/vga.h"
#include "arch/io.h"

/*
 * TODO(lesson 03). Questions to answer before you write code:
 *   - What's in each of the two bytes of a cell? Which byte comes first?
 *   - Why should the pointer to 0xB8000 be `volatile`?
 *   - The hardware cursor is set through the CRT controller's index/data
 *     port pair. Which ports, which register indexes?
 */

void vga_init(void) { }
void vga_clear(void) { }
void vga_set_color(enum vga_color fg, enum vga_color bg) { (void)fg; (void)bg; }
void vga_putc(char c) { (void)c; }
void vga_write(const char *s) { (void)s; }
