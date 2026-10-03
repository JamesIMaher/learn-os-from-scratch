# Lesson 10: The kernel heap

> **Goal:** `kmalloc` and `kfree` for arbitrary sizes, backed by pages you
> map on demand.
> **Files:** `src/mm/heap.c`
> **Checkpoint:** `make check L=10`

## Why this matters

Threads need stacks and bookkeeping structures, processes need tables, the
filesystem needs buffers, and none of them are exactly 4 KiB. A general
allocator is also a classic data structures problem with real trade-offs:
speed, wasted space, and fragmentation.

## Background

### Where the heap lives

`[KHEAP_START, KHEAP_END)` in kernel space. Initially nothing is mapped.
When you need more room, grab frames from the PMM and map them at the end of
the heap. (Because kernel page tables are shared, every address space sees
the heap.)

### Headers and blocks

The heap is a sequence of blocks, each used or free, each starting with a
small header that records at least its size and whether it's free. `kfree`
receives only a pointer, so the header must be findable from the pointer.

### Splitting and coalescing

If a free block is much bigger than a request, **split** it. When a block is
freed and its neighbor is free too, **coalesce** them into one, or after a
while the heap is a pile of small free fragments and large requests fail (or
the heap grows forever). Merging with the *next* block is easy. Merging with
the *previous* one needs a way to find it.

### Placement policies

First fit, next fit, best fit, segregated free lists... All work; they
trade search time against fragmentation. First fit is a fine start.

## Your mission

Implement `include/mm/heap.h`. All returned pointers must be 16-byte aligned.
Call `heap_init()` from `kmain` once paging is on.

## Questions before you code

1. What goes in your block header? How big is it? Does that keep the payload
   16-byte aligned?
2. From a `kfree(p)` pointer, how do you find the header? The next block? The
   previous block?
3. What's the smallest free block worth keeping after a split?
4. Allocate A, B, C (adjacent). Free A, then C, then B. Draw the heap after
   each step with your coalescing rules. Do you end with one free block?
5. `kcalloc(n, size)`: when can `n * size` overflow? How do you detect it
   without overflowing?
6. When the heap grows, if the last block is free, should the new pages
   extend it or become a new block?

## Milestones

1. `kmalloc` that only ever grows (a "bump allocator", no `kfree`).
2. `kfree` + first-fit reuse.
3. Splitting. Coalescing.
4. The stress test passes.

## The checkpoint verifies

Alignment, bounds and non-overlap for assorted sizes; exact `bytes_in_use`
bookkeeping; reuse of freed memory; coalescing in both directions (64
blocks freed in an interleaved order must merge enough to satisfy a larger
request without growing); a 1 MiB allocation; `kcalloc` zeroing and overflow
detection; 20000 random allocations and frees with content integrity
checks, and no growth beyond 4 MiB.

## Hints

<details><summary>Hint 1: a simple layout</summary>

A doubly linked list of blocks in address order: each header holds size,
a free flag, and next/prev pointers. On 32-bit that's 16 bytes, which keeps
payloads aligned if every size is rounded up to a multiple of 16.
</details>

<details><summary>Hint 2: an alternative to prev pointers</summary>

Boundary tags: put a copy of the block size in a *footer* at the end of each
block. The block before you ends right before your header, so its footer tells
you where it starts.
</details>

<details><summary>Hint 3: memory corruption in the stress test</summary>

Classic causes: forgetting to account for the header when splitting
(off by `sizeof(header)`), not updating the `prev` of the block after a
merged one, or a block's recorded size not including padding. Write a
`heap_check()` that walks every block and verifies the invariants (sizes add
up, links agree, no two adjacent free blocks). Call it after every operation
while debugging.
</details>

## Going further

- `krealloc`.
- Return whole free pages at the end of the heap to the PMM.
- Size-class "slab" caches for common sizes (thread structs, file handles).
- Detect double frees and writes past the end of a block with a canary.

## Reflect

- The heap can run while interrupts are enabled. Is it safe for an interrupt
  handler to call `kmalloc`? What about after lesson 11, when two threads
  might call it at once?

## References

- Knuth, TAOCP Vol. 1 §2.5 (dynamic storage allocation, boundary tags)
- Wilson et al., "Dynamic Storage Allocation: A Survey and Critical Review" (1995)
- OSDev wiki: "Heap", "Memory Allocation"
