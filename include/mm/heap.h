/*
 * heap.h - the kernel heap: kmalloc and friends.               [Lesson 10]
 *
 * Lives in virtual memory [KHEAP_START, KHEAP_END). Starts small and grows by
 * mapping new frames as needed. Freed memory must be reusable, and adjacent
 * free blocks must merge, or the heap fragments and grows forever.
 */
#pragma once
#include <stddef.h>

void  heap_init(void);

/* Returns a 16-byte-aligned pointer, or NULL. kmalloc(0) may return NULL. */
void *kmalloc(size_t size);
void *kcalloc(size_t count, size_t size);   /* zeroed; NULL on overflow too */
void  kfree(void *ptr);                     /* kfree(NULL) does nothing     */

struct heap_stats {
    size_t bytes_in_use;     /* sum of the sizes of live allocations (or more, if you round up) */
    size_t bytes_mapped;     /* how much of [KHEAP_START, KHEAP_END) is backed by frames */
};
void  heap_get_stats(struct heap_stats *out);
