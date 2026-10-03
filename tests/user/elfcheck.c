/* elfcheck - did the loader set up every kind of section correctly?
 * Exits with 42 if everything is right, or a small number naming the problem. */
#include "ulib.h"

static int initialized = 0x1234;             /* .data   */
static char zeroed[3 * 4096 + 100];          /* .bss: spans several pages */
static const char text[] = "read-only data"; /* .rodata */

static int check_stack(int depth)
{
    volatile char big[1024];                 /* uses ~40 KiB of stack in total */
    big[0] = (char)depth;
    big[1023] = (char)depth;
    if (depth == 0) return big[0] + big[1023];
    return check_stack(depth - 1) + big[0] - big[1023];
}

int main(void)
{
    if (initialized != 0x1234) return 1;
    for (unsigned i = 0; i < sizeof zeroed; i++) if (zeroed[i]) return 2;
    if (strcmp(text, "read-only data") != 0) return 3;
    initialized = 99;
    zeroed[sizeof zeroed - 1] = 1;           /* .data and .bss are writable */
    if (initialized != 99 || zeroed[sizeof zeroed - 1] != 1) return 4;
    if (check_stack(40) != 0) return 5;
    if (getpid() <= 0) return 6;
    char buf[32];
    snprintf(buf, sizeof buf, "%d-%s-%x", 12, "ab", 255);
    if (strcmp(buf, "12-ab-ff") != 0) return 7;
    puts("elfcheck-all-good");
    return 42;
}
