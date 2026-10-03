/*
 * pmm.h - physical memory manager.                             [Lesson 08]
 *
 * Hands out 4 KiB physical page frames. The bootloader tells you which RAM is
 * usable (the multiboot memory map). Some usable RAM is already in use and
 * must never be handed out:
 *   - anything below 1 MiB (keep it simple: just don't use low memory),
 *   - your kernel image: [_kernel_start, _kernel_end) from linker.ld,
 *   - every multiboot module (lessons 13-14 load programs and files from them),
 *   - the bootloader's own information that you'll still need later: the
 *     module list (mods_addr) and the command-line strings. Don't assume it
 *     all sits in low memory. Go and look where your bootloader put it.
 *   - whatever memory your allocator itself uses for bookkeeping.
 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

#define PAGE_SIZE 4096u

void      pmm_init(multiboot_info_t *mbi);

/* Physical address of a free, page-aligned frame, or 0 if memory is exhausted. */
uintptr_t pmm_alloc_frame(void);

/* Return a frame obtained from pmm_alloc_frame. */
void      pmm_free_frame(uintptr_t frame);

size_t    pmm_free_count(void);    /* frames currently free              */
size_t    pmm_total_count(void);   /* frames the allocator manages       */
