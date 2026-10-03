/*
 * vmm.h - virtual memory with x86 two-level paging.            [Lesson 09]
 *
 * The memory layout of every address space in this course:
 *
 *   0x00000000 .. KERNEL_SPACE_END   kernel space, supervisor-only, IDENTICAL in
 *                                    every address space. Physical RAM is
 *                                    identity mapped here (virtual == physical),
 *                                    and the kernel heap lives at KHEAP_START.
 *   USER_SPACE_START .. USER_SPACE_END   user space, private to each address space.
 *
 * Identity mapping all RAM makes life easy: any physical frame you get from
 * the PMM can be read and written through a pointer with the same value.
 * (Real kernels usually live in the "higher half" instead; see lesson 16.)
 *
 * The course assumes at most 256 MiB of RAM (the Makefile gives QEMU 128 MiB).
 */
#pragma once
#include <stdint.h>

#define KERNEL_SPACE_END 0x40000000u
#define KHEAP_START      0x20000000u
#define KHEAP_END        0x30000000u
#define USER_SPACE_START 0x40000000u
#define USER_SPACE_END   0xC0000000u

/* Mapping flags. Every mapping is readable; these add permissions. */
#define VMM_WRITE (1u << 0)
#define VMM_USER  (1u << 1)

typedef struct address_space address_space_t;   /* you define the struct */

/*
 * Build the kernel address space (identity map all RAM, supervisor-only,
 * writable), install a page-fault handler, load CR3 and turn paging on.
 * A page fault in kernel mode must panic, printing the faulting address
 * (where does the CPU put it?) and the decoded error code.
 */
void vmm_init(void);

address_space_t *vmm_kernel_space(void);
address_space_t *vmm_current(void);

/* A new address space: empty user space, kernel space shared with all others
 * (a kernel mapping added later, e.g. by the heap, must be visible in EVERY
 * address space, including ones created earlier). NULL if out of memory. */
address_space_t *vmm_create_space(void);

/* Free an address space: its page tables AND every frame mapped in its user
 * space. Must not be the current space. */
void vmm_destroy_space(address_space_t *as);

/* Load `as` into CR3. */
void vmm_switch(address_space_t *as);

/*
 * Map the page containing `virt` to the frame containing `phys` with `flags`.
 * Replaces any existing mapping for that page (and must make sure the CPU
 * doesn't keep using the old one). Allocates page tables as needed.
 * Returns 0, or -1 if out of memory.
 */
int  vmm_map(address_space_t *as, uintptr_t virt, uintptr_t phys, uint32_t flags);

/* Remove the mapping for the page containing `virt` (if any). Does NOT free the frame. */
void vmm_unmap(address_space_t *as, uintptr_t virt);

/*
 * Look up `virt`. Returns 0 and fills *phys_out (the exact physical address,
 * offset included) and *flags_out (VMM_* bits) if mapped; -1 if not mapped.
 * Either out-pointer may be NULL. Must work for any address space, not just
 * the current one.
 */
int  vmm_translate(address_space_t *as, uintptr_t virt, uintptr_t *phys_out, uint32_t *flags_out);
