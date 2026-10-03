/* gdt.c - see include/arch/gdt.h.                       [Lessons 04 and 12] */
#include "arch/gdt.h"

/*
 * TODO(lesson 04). You will need:
 *   - a C struct for one 8-byte segment descriptor. Look at the bit layout in
 *     the Intel SDM, Vol. 3A, section 3.4.5 ("Segment Descriptors"). Why are
 *     the base and limit chopped into pieces?
 *   - a C struct for the 6-byte operand of LGDT.
 *   - a little assembly (inline, or a new .asm file in src/arch/) to run LGDT
 *     and reload the segment registers.
 *
 * Use __attribute__((packed)) where the CPU expects an exact layout.
 */

void gdt_init(void) { }

/* TODO(lesson 12) */
void tss_init(void) { }
void tss_set_kernel_stack(uint32_t esp0) { (void)esp0; }
