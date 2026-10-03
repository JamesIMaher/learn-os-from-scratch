/* Lesson 10 checkpoints: the kernel heap. */
#include "harness.h"
#include "mm/heap.h"
#include "mm/vmm.h"

static bool in_heap(void *p, size_t n)
{
    uintptr_t a = (uintptr_t)p;
    return a >= KHEAP_START && a + n <= KHEAP_END;
}

static void fill(void *p, size_t n, uint8_t v) { for (size_t i = 0; i < n; i++) ((uint8_t *)p)[i] = v; }
static bool check(void *p, size_t n, uint8_t v)
{
    for (size_t i = 0; i < n; i++) if (((uint8_t *)p)[i] != v) return false;
    return true;
}

#define SLOTS 128
static void *slot_p[SLOTS];
static size_t slot_n[SLOTS];

void lesson10(void)
{
    t_lesson(10, "The kernel heap");
    heap_init();
    struct heap_stats s0, s;
    heap_get_stats(&s0);

    t_begin("kmalloc returns aligned, non-overlapping heap memory");
    static const size_t sizes[] = { 1, 13, 16, 100, 4096, 10000, 3 };
    void *p[7];
    bool ok = true;
    for (int i = 0; i < 7; i++) {
        p[i] = kmalloc(sizes[i]);
        if (!p[i] || !in_heap(p[i], sizes[i]) || (uintptr_t)p[i] % 16) ok = false;
        else fill(p[i], sizes[i], (uint8_t)(0x40 + i));
    }
    CHECK(ok);
    for (int i = 0; i < 7 && ok; i++) CHECK(check(p[i], sizes[i], (uint8_t)(0x40 + i)));
    heap_get_stats(&s);
    CHECK(s.bytes_in_use >= s0.bytes_in_use + 14229);
    CHECK(s.bytes_mapped >= s.bytes_in_use);
    for (int i = 0; i < 7; i++) kfree(p[i]);
    heap_get_stats(&s);
    CHECK_EQ(s.bytes_in_use, s0.bytes_in_use);
    kfree(0);
    t_end();

    t_begin("freed memory is reused");
    void *a = kmalloc(200);
    kfree(a);
    void *b = kmalloc(200);
    CHECK(a == b);
    kfree(b);
    t_end();

    t_begin("adjacent free blocks merge (no fragmentation growth)");
    void *blk[64];
    for (int i = 0; i < 64; i++) blk[i] = kmalloc(1000);
    heap_get_stats(&s);
    size_t mapped = s.bytes_mapped;
    for (int i = 0; i < 64; i += 2) kfree(blk[i]);    /* evens, then odds: */
    for (int i = 1; i < 64; i += 2) kfree(blk[i]);    /* merges both ways  */
    void *big = kmalloc(60000);
    CHECK(big != 0);
    heap_get_stats(&s);
    CHECK(s.bytes_mapped <= mapped);
    kfree(big);
    t_end();

    t_begin("large allocations grow the heap");
    void *huge = kmalloc(1024 * 1024);
    CHECK(huge != 0);
    if (huge) {
        CHECK(in_heap(huge, 1024 * 1024));
        fill(huge, 1024 * 1024, 0x5A);
        CHECK(check(huge, 1024 * 1024, 0x5A));
        kfree(huge);
    }
    t_end();

    t_begin("kcalloc zeroes memory and catches overflow");
    void *dirty = kmalloc(512);
    fill(dirty, 512, 0xFF);
    kfree(dirty);
    void *clean = kcalloc(8, 64);
    CHECK(clean != 0);
    if (clean) CHECK(check(clean, 512, 0));
    kfree(clean);
    CHECK(kcalloc(0x10000, 0x10001) == 0);
    t_end();

    t_begin("stress: 20000 random kmalloc/kfree with integrity checks");
    for (int i = 0; i < SLOTS; i++) { slot_p[i] = 0; slot_n[i] = 0; }
    bool good = true;
    for (int op = 0; op < 20000 && good; op++) {
        int i = (int)(t_rand() % SLOTS);
        if (slot_p[i]) {
            if (!check(slot_p[i], slot_n[i], (uint8_t)i)) good = false;
            kfree(slot_p[i]);
            slot_p[i] = 0;
        } else {
            slot_n[i] = 1 + t_rand() % 3000;
            slot_p[i] = kmalloc(slot_n[i]);
            if (!slot_p[i] || (uintptr_t)slot_p[i] % 16) good = false;
            else fill(slot_p[i], slot_n[i], (uint8_t)i);
        }
    }
    CHECK(good);
    for (int i = 0; i < SLOTS; i++) if (slot_p[i]) kfree(slot_p[i]);
    heap_get_stats(&s);
    CHECK_EQ(s.bytes_in_use, s0.bytes_in_use);
    CHECK(s.bytes_mapped <= 4 * 1024 * 1024);
    t_end();
}
