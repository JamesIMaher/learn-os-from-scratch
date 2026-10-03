/*
 * harness.h - the checkpoint test framework.
 *
 * A word about reading the tests: they are a SPECIFICATION. Reading what a
 * test checks is fair game and often clarifies the lesson. But a few tests
 * have to poke hardware directly to see what your code did, so they contain
 * small spoilers. If you want to discover things yourself, read the CHECKs,
 * not the plumbing around them.
 *
 * The harness talks to tools/check.py over QEMU's "debugcon" port (0xE9),
 * NOT over your serial driver, so it works even when your drivers don't.
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

/* ---- output to the harness ------------------------------------------- */
void t_putc(char c);
void t_puts(const char *s);
void t_putdec(int32_t v);
void t_puthex(uint32_t v);

/* ---- test structure ---------------------------------------------------- */
void t_lesson(int n, const char *title);
void t_begin(const char *name);
void t_end(void);
void t_check(bool ok, const char *expr, const char *file, int line);
void t_check_eq(uint32_t got, uint32_t want, const char *expr, const char *wexpr,
                const char *file, int line);
void t_check_str(const char *got, const char *want, const char *expr,
                 const char *file, int line);
void t_note(const char *msg);

#define CHECK(c)          t_check(!!(c), #c, __FILE__, __LINE__)
#define CHECK_EQ(a, b)    t_check_eq((uint32_t)(a), (uint32_t)(b), #a, #b, __FILE__, __LINE__)
#define CHECK_STR(a, b)   t_check_str((a), (b), #a, __FILE__, __LINE__)

/* Ask check.py to verify that `token` shows up in your serial output by the
 * end of the run. "\\r" and "\\n" in the token are escapes. */
void t_expect_serial(const char *token);

/* Ask check.py to type keys (QEMU "sendkey" names, space separated). */
void t_send_keys(const char *keys);

/* ---- helpers ------------------------------------------------------------ */
extern uint32_t          t_magic;
extern multiboot_info_t *t_mbi;
const multiboot_module_t *t_find_module(const char *suffix);
bool t_streq(const char *a, const char *b);
size_t t_strlen(const char *s);
/* Tiny decimal formatting for building tokens: appends v to buf at *pos. */
void t_fmt_str(char *buf, size_t *pos, const char *s);
void t_fmt_dec(char *buf, size_t *pos, int32_t v);
/* Busy-wait (no interrupts needed) for roughly `iterations` spins. */
void t_spin(uint32_t iterations);
/* Wait up to `ms` (using timer_ticks) until *flag != 0. Returns true if set. */
bool t_wait_flag(volatile int *flag, uint32_t ms);
uint32_t t_rand(void);
/* Leak detection that tolerates the heap keeping memory it grew into. */
struct t_mem { size_t heap_in_use, heap_mapped, free_frames; };
void t_mem_snapshot(struct t_mem *m);
/* true if, since `before`, less than `slack` bytes went missing */
bool t_mem_no_leak(const struct t_mem *before, size_t slack);

static inline void t_outb(uint16_t port, uint8_t v) { __asm__ volatile("outb %0, %1" :: "a"(v), "Nd"(port)); }
static inline uint8_t t_inb(uint16_t port) { uint8_t v; __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }
static inline uint32_t t_eflags(void) { uint32_t f; __asm__ volatile("pushf; pop %0" : "=r"(f)); return f; }
static inline uint32_t t_cr0(void) { uint32_t v; __asm__ volatile("mov %%cr0, %0" : "=r"(v)); return v; }
static inline uint32_t t_cr3(void) { uint32_t v; __asm__ volatile("mov %%cr3, %0" : "=r"(v)); return v; }

/* One function per lesson; runner.c runs lessons 1..N in order. */
void lesson01(void); void lesson02(void); void lesson03(void); void lesson04(void);
void lesson05(void); void lesson06(void); void lesson07(void); void lesson08(void);
void lesson09(void); void lesson10(void); void lesson11(void); void lesson12(void);
void lesson13(void); void lesson14(void); void lesson15(void);
