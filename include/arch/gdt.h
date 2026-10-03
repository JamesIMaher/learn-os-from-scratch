/*
 * gdt.h - the Global Descriptor Table.                  [Lessons 04 and 12]
 *
 * The rest of the kernel depends on these selector values, so your GDT must
 * put the descriptors in exactly these slots:
 *
 *   slot 0  null descriptor (required by the CPU)
 *   slot 1  kernel code: base 0, limit 4 GiB, ring 0, 32-bit, execute/read
 *   slot 2  kernel data: base 0, limit 4 GiB, ring 0, 32-bit, read/write
 *   slot 3  user code:   same as kernel code but ring 3
 *   slot 4  user data:   same as kernel data but ring 3
 *   slot 5  TSS (left empty until lesson 12, but reserve the slot now)
 *
 * Why are the user selectors 0x1B and 0x23 instead of 0x18 and 0x20?
 */
#pragma once
#include <stdint.h>

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x1B
#define GDT_USER_DATA   0x23
#define GDT_TSS         0x28

/*
 * Build the GDT, load it with LGDT, and reload EVERY segment register so the
 * CPU actually uses it (CS needs special treatment: why can't you just MOV?).
 */
void gdt_init(void);

/* ---- Lesson 12 ---------------------------------------------------------- */

/* Fill in the TSS descriptor (slot 5) and load the task register (LTR). */
void tss_init(void);

/* Set the stack the CPU switches to when an interrupt arrives in ring 3. */
void tss_set_kernel_stack(uint32_t esp0);
