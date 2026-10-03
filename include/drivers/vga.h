/*
 * vga.h - VGA text-mode console (80x25).                        [Lesson 03]
 *
 * The screen is a grid of 80x25 cells memory-mapped at physical 0xB8000.
 * Each cell is two bytes. Figuring out what those two bytes mean is the
 * first task of the lesson.
 */
#pragma once
#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

enum vga_color {
    VGA_BLACK = 0, VGA_BLUE, VGA_GREEN, VGA_CYAN, VGA_RED, VGA_MAGENTA,
    VGA_BROWN, VGA_LIGHT_GREY, VGA_DARK_GREY, VGA_LIGHT_BLUE, VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN, VGA_LIGHT_RED, VGA_LIGHT_MAGENTA, VGA_YELLOW, VGA_WHITE,
};

/* Set the default color (light grey on black), clear the screen, cursor to 0,0. */
void vga_init(void);

/* Fill every cell with a space in the CURRENT color; cursor to 0,0. */
void vga_clear(void);

/* Colors used for characters written from now on. */
void vga_set_color(enum vga_color fg, enum vga_color bg);

/*
 * Draw one character at the cursor and advance. Must handle:
 *   '\n'  move to the start of the next line
 *   '\r'  move to the start of the current line
 *   '\t'  advance to the next column that is a multiple of 8
 *   '\b'  move back one cell (not past column 0) and blank it
 *   wrapping at column 80, and scrolling when the cursor leaves row 24:
 *   every row moves up by one and the new bottom row is blank (current color).
 * The blinking hardware cursor must follow the logical cursor.
 */
void vga_putc(char c);
void vga_write(const char *s);
