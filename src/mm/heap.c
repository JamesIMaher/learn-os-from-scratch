/* heap.c - see include/mm/heap.h.                               [Lesson 10] */
#include "mm/heap.h"
#include "mm/vmm.h"
#include "mm/pmm.h"

/*
 * TODO(lesson 10). The classic design: every block (free or used) starts with
 * a small header recording its size and whether it's free. Questions:
 *   - Given a pointer passed to kfree, how do you find its header?
 *   - How do you find the NEXT block? The PREVIOUS one? (Merging needs both.)
 *   - How do you keep every returned pointer 16-byte aligned?
 *   - When no free block fits, how do you grow?
 */

void heap_init(void) { }
void *kmalloc(size_t size) { (void)size; return 0; }
void *kcalloc(size_t count, size_t size) { (void)count; (void)size; return 0; }
void kfree(void *ptr) { (void)ptr; }
void heap_get_stats(struct heap_stats *out) { out->bytes_in_use = 0; out->bytes_mapped = 0; }
