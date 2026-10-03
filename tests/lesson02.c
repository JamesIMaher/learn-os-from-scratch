/* Lesson 02 checkpoints: port I/O, serial, a tiny libc, printf. */
#include "harness.h"
#include "lib/string.h"
#include "lib/printf.h"
#include "drivers/serial.h"
#include "kernel/console.h"

static char buf[256];

static int fmt(const char *f, ...)
{
    va_list ap;
    va_start(ap, f);
    for (size_t i = 0; i < sizeof buf; i++) buf[i] = '#';
    int r = kvsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return r;
}

static void test_mem(void)
{
    static unsigned char a[64], b[64];

    t_begin("memset fills exactly n bytes and returns dst");
    for (int i = 0; i < 64; i++) a[i] = 0x11;
    CHECK(memset(a + 8, 0xAB, 16) == a + 8);
    CHECK_EQ(a[7], 0x11); CHECK_EQ(a[8], 0xAB); CHECK_EQ(a[23], 0xAB); CHECK_EQ(a[24], 0x11);
    memset(a, 0x1FF, 1);                 /* value is converted to unsigned char */
    CHECK_EQ(a[0], 0xFF);
    t_end();

    t_begin("memcpy copies exactly n bytes and returns dst");
    for (int i = 0; i < 64; i++) { a[i] = (unsigned char)i; b[i] = 0; }
    CHECK(memcpy(b + 1, a, 33) == b + 1);
    CHECK_EQ(b[0], 0); CHECK_EQ(b[1], 0); CHECK_EQ(b[33], 32); CHECK_EQ(b[34], 0);
    t_end();

    t_begin("memmove handles overlap in both directions");
    for (int i = 0; i < 64; i++) a[i] = (unsigned char)i;
    memmove(a + 4, a, 20);               /* dst after src */
    CHECK_EQ(a[4], 0); CHECK_EQ(a[23], 19); CHECK_EQ(a[3], 3);
    for (int i = 0; i < 64; i++) a[i] = (unsigned char)i;
    memmove(a, a + 4, 20);               /* dst before src */
    CHECK_EQ(a[0], 4); CHECK_EQ(a[19], 23); CHECK_EQ(a[20], 20);
    t_end();

    t_begin("memcmp compares as unsigned bytes");
    CHECK(memcmp("abc", "abc", 3) == 0);
    CHECK(memcmp("abd", "abc", 3) > 0);
    CHECK(memcmp("abc", "abd", 3) < 0);
    CHECK(memcmp("\x80", "\x01", 1) > 0);
    CHECK(memcmp("xyz", "abc", 0) == 0);
    t_end();

    t_begin("strlen, strcmp, strncmp");
    CHECK_EQ(strlen(""), 0); CHECK_EQ(strlen("hello"), 5);
    CHECK(strcmp("abc", "abc") == 0);
    CHECK(strcmp("abc", "abd") < 0);
    CHECK(strcmp("abc", "ab") > 0);
    CHECK(strcmp("", "a") < 0);
    CHECK(strcmp("\xff", "a") > 0);
    CHECK(strncmp("abcdef", "abcxyz", 3) == 0);
    CHECK(strncmp("abcdef", "abcxyz", 4) < 0);
    CHECK(strncmp("ab", "abc", 5) < 0);
    CHECK(strncmp("x", "y", 0) == 0);
    t_end();

    t_begin("strcpy, strncpy, strchr");
    char s[16];
    for (int i = 0; i < 16; i++) s[i] = 'Z';
    CHECK(strcpy(s, "hey") == s);
    CHECK_STR(s, "hey"); CHECK_EQ(s[4], 'Z');
    for (int i = 0; i < 16; i++) s[i] = 'Z';
    strncpy(s, "hi", 6);                 /* pads with NULs up to n */
    CHECK_EQ(s[0], 'h'); CHECK_EQ(s[2], 0); CHECK_EQ(s[5], 0); CHECK_EQ(s[6], 'Z');
    for (int i = 0; i < 16; i++) s[i] = 'Z';
    strncpy(s, "abcdef", 3);             /* does NOT terminate if src is too long */
    CHECK_EQ(s[2], 'c'); CHECK_EQ(s[3], 'Z');
    const char *h = "hello";
    CHECK(strchr(h, 'l') == h + 2);
    CHECK(strchr(h, 'z') == 0);
    CHECK(strchr(h, 0) == h + 5);
    t_end();
}

static void test_printf(void)
{
    int r;
    t_begin("kvsnprintf: plain text and %%");
    r = fmt("hello"); CHECK_STR(buf, "hello"); CHECK_EQ(r, 5);
    r = fmt("100%%"); CHECK_STR(buf, "100%"); CHECK_EQ(r, 4);
    r = fmt(""); CHECK_STR(buf, ""); CHECK_EQ(r, 0);
    t_end();

    t_begin("kvsnprintf: %c and %s");
    fmt("[%c]", 'x'); CHECK_STR(buf, "[x]");
    fmt("[%s|%s]", "ab", ""); CHECK_STR(buf, "[ab|]");
    fmt("%s", (char *)0); CHECK_STR(buf, "(null)");
    t_end();

    t_begin("kvsnprintf: %d %i %u");
    fmt("%d", 0); CHECK_STR(buf, "0");
    fmt("%d", 12345); CHECK_STR(buf, "12345");
    fmt("%i", -42); CHECK_STR(buf, "-42");
    fmt("%d", (int)0x80000000); CHECK_STR(buf, "-2147483648");
    fmt("%d", 2147483647); CHECK_STR(buf, "2147483647");
    fmt("%u", 4294967295u); CHECK_STR(buf, "4294967295");
    fmt("%ld", 7L); CHECK_STR(buf, "7");
    t_end();

    t_begin("kvsnprintf: %x %X %p");
    fmt("%x", 0); CHECK_STR(buf, "0");
    fmt("%x", 0xdeadbeef); CHECK_STR(buf, "deadbeef");
    fmt("%X", 0xdeadbeef); CHECK_STR(buf, "DEADBEEF");
    fmt("%x", 255); CHECK_STR(buf, "ff");
    fmt("%p", (void *)0xb8000); CHECK_STR(buf, "0x000b8000");
    t_end();

    t_begin("kvsnprintf: width, '0' and '-' flags");
    fmt("[%5d]", 42); CHECK_STR(buf, "[   42]");
    fmt("[%-5d]", 42); CHECK_STR(buf, "[42   ]");
    fmt("[%05d]", 42); CHECK_STR(buf, "[00042]");
    fmt("[%08x]", 0xbeef); CHECK_STR(buf, "[0000beef]");
    fmt("[%6s]", "ab"); CHECK_STR(buf, "[    ab]");
    fmt("[%-6s]", "ab"); CHECK_STR(buf, "[ab    ]");
    fmt("[%2d]", 12345); CHECK_STR(buf, "[12345]");
    fmt("[%3c]", 'z'); CHECK_STR(buf, "[  z]");
    fmt("[%05d]", -42); CHECK_STR(buf, "[-0042]");
    t_end();

    t_begin("kvsnprintf: several conversions together");
    fmt("%s=%d (0x%x) %c%%", "answer", 42, 42, '!');
    CHECK_STR(buf, "answer=42 (0x2a) !%");
    t_end();

    t_begin("kvsnprintf: truncation and return value");
    char small[8];
    for (int i = 0; i < 8; i++) small[i] = '#';
    r = ksnprintf(small, 6, "hello world");
    CHECK_EQ(r, 11);                     /* what it WOULD have written */
    CHECK_STR(small, "hello");           /* 5 chars + NUL = 6 bytes    */
    CHECK_EQ(small[6], '#');
    small[0] = '#';
    r = ksnprintf(small, 0, "abc");      /* size 0: write nothing at all */
    CHECK_EQ(r, 3);
    CHECK_EQ(small[0], '#');
    r = ksnprintf(small, 1, "abc");
    CHECK_EQ(small[0], 0);
    t_end();
}

void lesson02(void)
{
    t_lesson(2, "Port I/O, serial, and a tiny libc");
    test_mem();
    test_printf();

    t_begin("serial_init passes its loopback self-test");
    CHECK_EQ(serial_init(), 0);
    t_end();

    t_begin("serial output reaches the outside world");
    serial_write("serial-token-7f3a\n");
    t_expect_serial("serial-token-7f3a");
    serial_putc('c'); serial_putc('r'); serial_putc('l'); serial_putc('f'); serial_putc('\n');
    t_expect_serial("crlf\\r\\n");
    t_end();

    t_begin("kprintf goes to the console");
    kprintf("kprintf-%d-%s-%x\n", 42, "ok", 0xc0de);
    t_expect_serial("kprintf-42-ok-c0de");
    console_write("console-write-ok\n");
    t_expect_serial("console-write-ok");
    t_end();
}
