/* Lesson 03 checkpoints: VGA text mode. */
#include "harness.h"
#include "drivers/vga.h"
#include "kernel/console.h"

#define CELL(r, c) (((volatile uint16_t *)0xB8000)[(r) * VGA_WIDTH + (c)])
#define ATTR(fg, bg) ((uint16_t)(((bg) << 4) | (fg)) << 8)

/* Read the hardware cursor position back from the CRT controller. */
static int hw_cursor(void)
{
    t_outb(0x3D4, 0x0F);
    int pos = t_inb(0x3D5);
    t_outb(0x3D4, 0x0E);
    pos |= t_inb(0x3D5) << 8;
    return pos;
}

static bool row_is(int r, const char *text, uint16_t attr)
{
    int n = (int)t_strlen(text);
    for (int c = 0; c < VGA_WIDTH; c++) {
        uint16_t want = attr | (uint8_t)(c < n ? text[c] : ' ');
        if (CELL(r, c) != want) return false;
    }
    return true;
}

void lesson03(void)
{
    t_lesson(3, "VGA text mode");
    const uint16_t grey = ATTR(VGA_LIGHT_GREY, VGA_BLACK);

    t_begin("vga_init clears the screen to light grey on black");
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) CELL(0, i) = 0x4141;
    vga_init();
    bool all = true;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) all = all && CELL(0, i) == (grey | ' ');
    CHECK(all);
    CHECK_EQ(CELL(0, 0), grey | ' ');
    CHECK_EQ(CELL(24, 79), grey | ' ');
    CHECK_EQ(hw_cursor(), 0);
    t_end();

    t_begin("characters and colors land in the right cells");
    vga_write("ok");
    CHECK_EQ(CELL(0, 0), grey | 'o');
    CHECK_EQ(CELL(0, 1), grey | 'k');
    vga_set_color(VGA_WHITE, VGA_BLUE);
    vga_putc('H');
    vga_putc('i');
    CHECK_EQ(CELL(0, 2), ATTR(VGA_WHITE, VGA_BLUE) | 'H');
    CHECK_EQ(CELL(0, 3), ATTR(VGA_WHITE, VGA_BLUE) | 'i');
    CHECK_EQ(hw_cursor(), 4);
    t_end();

    t_begin("vga_clear uses the current color");
    vga_set_color(VGA_GREEN, VGA_BLACK);
    vga_clear();
    CHECK_EQ(CELL(0, 0), ATTR(VGA_GREEN, VGA_BLACK) | ' ');
    CHECK_EQ(CELL(12, 40), ATTR(VGA_GREEN, VGA_BLACK) | ' ');
    CHECK_EQ(hw_cursor(), 0);
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
    t_end();

    t_begin("newline, carriage return, tab, backspace");
    vga_write("abc\n");
    CHECK_EQ(hw_cursor(), 80);
    vga_write("xyz\rQ");
    CHECK_EQ(CELL(1, 0), grey | 'Q');
    CHECK_EQ(CELL(1, 1), grey | 'y');
    CHECK_EQ(hw_cursor(), 81);
    vga_putc('\t');
    CHECK_EQ(hw_cursor(), 88);
    vga_putc('\t');
    CHECK_EQ(hw_cursor(), 96);
    vga_write("\rabc\b");
    CHECK_EQ(hw_cursor(), 82);
    CHECK_EQ(CELL(1, 2), grey | ' ');
    CHECK_EQ(CELL(1, 1), grey | 'b');
    vga_write("\r\b\b");
    CHECK_EQ(hw_cursor(), 80);          /* never moves left of column 0 */
    t_end();

    t_begin("long lines wrap to the next row");
    vga_clear();
    for (int i = 0; i < 81; i++) vga_putc((char)('A' + i % 26));
    CHECK_EQ(CELL(0, 79), grey | ('A' + 79 % 26));
    CHECK_EQ(CELL(1, 0), grey | ('A' + 80 % 26));
    CHECK_EQ(hw_cursor(), 81);
    t_end();

    t_begin("output past the last row scrolls the screen up");
    vga_clear();
    char line[4] = { 'L', 0, 0, 0 };
    for (int r = 0; r < VGA_HEIGHT; r++) {
        line[1] = (char)('A' + r);
        vga_write(line);
        if (r != VGA_HEIGHT - 1) vga_putc('\n');
    }
    CHECK(row_is(0, "LA", grey));
    CHECK(row_is(24, "LY", grey));
    vga_set_color(VGA_YELLOW, VGA_RED);
    vga_write("\nZ");
    CHECK(row_is(0, "LB", grey));
    CHECK(row_is(23, "LY", grey));
    CHECK_EQ(CELL(24, 0), ATTR(VGA_YELLOW, VGA_RED) | 'Z');
    CHECK_EQ(CELL(24, 1), ATTR(VGA_YELLOW, VGA_RED) | ' ');
    CHECK_EQ(CELL(24, 79), ATTR(VGA_YELLOW, VGA_RED) | ' ');
    CHECK_EQ(hw_cursor(), 24 * 80 + 1);
    for (int i = 0; i < 80 * 3; i++) vga_putc('.');   /* scroll several times */
    CHECK_EQ(hw_cursor(), 24 * 80 + 1);
    CHECK_EQ(CELL(20, 0), grey | 'L');
    CHECK_EQ(CELL(20, 1), grey | 'Y');
    t_end();

    t_begin("the console now also draws on the screen");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
    console_write("con");
    CHECK_EQ(CELL(0, 0), grey | 'c');
    CHECK_EQ(CELL(0, 2), grey | 'n');
    vga_clear();
    t_end();
}
