/*
 * runner.c - the test kernel's entry point.
 *
 * In the test build your kmain() is renamed and boot.asm calls THIS kmain.
 * It reads "lesson=N" from the multiboot command line and runs the
 * checkpoints for lessons 1..N in order, each lesson bringing up its own
 * subsystem. (So lesson N's tests run on top of your lessons 1..N-1.)
 */
#include "harness.h"
#include "drivers/timer.h"
#include "mm/heap.h"
#include "mm/pmm.h"

uint32_t          t_magic;
multiboot_info_t *t_mbi;

#define DEBUGCON 0xE9
#define EXIT_PORT 0xF4

static const char *cur_test;
static int cur_fails, total_pass, total_fail;

void t_putc(char c) { t_outb(DEBUGCON, (uint8_t)c); }
void t_puts(const char *s) { while (*s) t_putc(*s++); }
void t_putdec(int32_t v)
{
    char tmp[12];
    int i = 0;
    uint32_t u = v < 0 ? -(uint32_t)v : (uint32_t)v;
    if (v < 0) t_putc('-');
    do { tmp[i++] = (char)('0' + u % 10); u /= 10; } while (u);
    while (i) t_putc(tmp[--i]);
}
void t_puthex(uint32_t v)
{
    t_puts("0x");
    for (int s = 28; s >= 0; s -= 4) t_putc("0123456789abcdef"[(v >> s) & 15]);
}

size_t t_strlen(const char *s) { size_t n = 0; while (s && s[n]) n++; return n; }
bool t_streq(const char *a, const char *b)
{
    if (!a || !b) return a == b;
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}
void t_fmt_str(char *buf, size_t *pos, const char *s) { while (*s) buf[(*pos)++] = *s++; buf[*pos] = 0; }
void t_fmt_dec(char *buf, size_t *pos, int32_t v)
{
    char tmp[12];
    int i = 0;
    uint32_t u = v < 0 ? -(uint32_t)v : (uint32_t)v;
    if (v < 0) buf[(*pos)++] = '-';
    do { tmp[i++] = (char)('0' + u % 10); u /= 10; } while (u);
    while (i) buf[(*pos)++] = tmp[--i];
    buf[*pos] = 0;
}

static uint32_t rng = 0x2545F491;
uint32_t t_rand(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

void t_spin(uint32_t n) { for (volatile uint32_t i = 0; i < n; i++) __asm__ volatile("pause"); }

bool t_wait_flag(volatile int *flag, uint32_t ms)
{
    uint64_t start = timer_ticks();
    uint32_t hz = timer_hz() ? timer_hz() : 100;
    uint64_t limit = (uint64_t)ms * hz / 1000 + 1;
    for (uint32_t spins = 0; !*flag; spins++) {
        if (timer_ticks() - start > limit) return false;
        if (spins > 400000000u) return false;    /* timer broken? don't hang */
        __asm__ volatile("pause");
    }
    return true;
}

void t_mem_snapshot(struct t_mem *m)
{
    struct heap_stats h;
    heap_get_stats(&h);
    m->heap_in_use = h.bytes_in_use;
    m->heap_mapped = h.bytes_mapped;
    m->free_frames = pmm_free_count();
}

bool t_mem_no_leak(const struct t_mem *b, size_t slack)
{
    struct t_mem a;
    t_mem_snapshot(&a);
    size_t heap_growth = a.heap_mapped > b->heap_mapped ? a.heap_mapped - b->heap_mapped : 0;
    size_t frames_gone = a.free_frames < b->free_frames ? (b->free_frames - a.free_frames) * 4096 : 0;
    size_t in_use_growth = a.heap_in_use > b->heap_in_use ? a.heap_in_use - b->heap_in_use : 0;
    bool ok = in_use_growth <= slack && frames_gone <= heap_growth + slack;
    if (!ok) {
        t_puts("@@NOTE leak check: heap in use grew by "); t_putdec((int32_t)in_use_growth);
        t_puts(" bytes; free frames dropped by "); t_putdec((int32_t)(frames_gone / 4096));
        t_puts(" while the heap grew by "); t_putdec((int32_t)(heap_growth / 4096)); t_puts(" pages\n");
    }
    return ok;
}

void t_lesson(int n, const char *title)
{
    t_puts("@@LESSON "); t_putdec(n); t_putc(' '); t_puts(title); t_putc('\n');
}

void t_begin(const char *name)
{
    cur_test = name;
    cur_fails = 0;
    t_puts("@@BEGIN "); t_puts(name); t_putc('\n');
}

void t_end(void)
{
    if (cur_fails) { total_fail++; t_puts("@@FAILED "); }
    else { total_pass++; t_puts("@@PASSED "); }
    t_puts(cur_test); t_putc('\n');
}

static void fail_prefix(const char *file, int line)
{
    cur_fails++;
    t_puts("@@FAIL "); t_puts(file); t_putc(':'); t_putdec(line); t_puts(": ");
}

void t_check(bool ok, const char *expr, const char *file, int line)
{
    if (ok) return;
    if (cur_fails >= 6) { cur_fails++; return; }
    fail_prefix(file, line);
    t_puts("expected "); t_puts(expr); t_putc('\n');
}

void t_check_eq(uint32_t got, uint32_t want, const char *expr, const char *wexpr,
                const char *file, int line)
{
    if (got == want) return;
    if (cur_fails >= 6) { cur_fails++; return; }
    fail_prefix(file, line);
    t_puts(expr); t_puts(" is "); t_puthex(got); t_puts(" ("); t_putdec((int32_t)got);
    t_puts("), expected "); t_puts(wexpr); t_puts(" = "); t_puthex(want);
    t_puts(" ("); t_putdec((int32_t)want); t_puts(")\n");
}

static void put_quoted(const char *s)
{
    t_putc('"');
    for (int i = 0; s && s[i] && i < 80; i++) {
        char c = s[i];
        if (c == '\n') t_puts("\\n");
        else if (c == '\r') t_puts("\\r");
        else if (c == '\t') t_puts("\\t");
        else if (c == '\b') t_puts("\\b");
        else if (c < 32 || c > 126) { t_puts("\\x"); t_putc("0123456789abcdef"[(c >> 4) & 15]); t_putc("0123456789abcdef"[c & 15]); }
        else t_putc(c);
    }
    t_putc('"');
}

void t_check_str(const char *got, const char *want, const char *expr, const char *file, int line)
{
    if (t_streq(got, want)) return;
    if (cur_fails >= 6) { cur_fails++; return; }
    fail_prefix(file, line);
    t_puts(expr); t_puts(" is "); if (got) put_quoted(got); else t_puts("NULL");
    t_puts(", expected "); put_quoted(want); t_putc('\n');
}

void t_note(const char *msg) { t_puts("@@NOTE "); t_puts(msg); t_putc('\n'); }
void t_expect_serial(const char *token) { t_puts("@@EXPECT-SERIAL "); t_puts(token); t_putc('\n'); }
void t_send_keys(const char *keys) { t_puts("@@SENDKEYS "); t_puts(keys); t_putc('\n'); }

const multiboot_module_t *t_find_module(const char *suffix)
{
    if (!t_mbi || !(t_mbi->flags & MULTIBOOT_INFO_MODS)) return 0;
    const multiboot_module_t *mods = (const multiboot_module_t *)(uintptr_t)t_mbi->mods_addr;
    size_t sl = t_strlen(suffix);
    for (uint32_t i = 0; i < t_mbi->mods_count; i++) {
        const char *c = (const char *)(uintptr_t)mods[i].cmdline;
        /* the command line may carry arguments after the file name */
        for (size_t j = 0; c && c[j]; j++) {
            size_t k = 0;
            while (k < sl && c[j + k] == suffix[k]) k++;
            if (k == sl && (c[j + k] == 0 || c[j + k] == ' ')) return &mods[i];
        }
    }
    return 0;
}

static int parse_lesson(void)
{
    if (!t_mbi || !(t_mbi->flags & MULTIBOOT_INFO_CMDLINE)) return 1;
    const char *c = (const char *)(uintptr_t)t_mbi->cmdline;
    for (; c && *c; c++) {
        if (c[0] == 'l' && c[1] == 'e' && c[2] == 's' && c[3] == 's' &&
            c[4] == 'o' && c[5] == 'n' && c[6] == '=') {
            int n = 0;
            for (c += 7; *c >= '0' && *c <= '9'; c++) n = n * 10 + (*c - '0');
            return n;
        }
    }
    return 1;
}

static void (*const lessons[])(void) = {
    lesson01, lesson02, lesson03, lesson04, lesson05, lesson06, lesson07, lesson08,
    lesson09, lesson10, lesson11, lesson12, lesson13, lesson14, lesson15,
};
#define NLESSONS ((int)(sizeof lessons / sizeof lessons[0]))

static void finish(void)
{
    t_puts("@@DONE "); t_putdec(total_pass); t_putc(' '); t_putdec(total_fail); t_putc('\n');
    t_outb(EXIT_PORT, total_fail ? 1 : 0);    /* isa-debug-exit: QEMU quits */
    for (;;) __asm__ volatile("cli; hlt");
}

void kmain(uint32_t magic, multiboot_info_t *mbi)
{
    t_magic = magic;
    t_mbi = magic == MULTIBOOT_BOOTLOADER_MAGIC ? mbi : 0;
    int target = parse_lesson();
    t_puts("@@START "); t_putdec(target); t_putc('\n');
    for (int i = 0; i < target && i < NLESSONS; i++) lessons[i]();
    finish();
}
