/* idt.c - see include/arch/idt.h.                               [Lesson 05] */
#include "arch/idt.h"

/*
 * TODO(lesson 05). The pieces:
 *   - a C struct for an 8-byte interrupt gate (Intel SDM Vol. 3A, 6.11),
 *   - a table of 256 of them, and the LIDT operand,
 *   - one tiny assembly entry stub per vector (see isr.asm) that all funnel
 *     into a common stub, which saves state and calls...
 *   - ...your C dispatcher, which looks up the registered handler.
 */

void idt_init(void) { }

isr_handler_t isr_register(uint8_t vector, isr_handler_t handler)
{
    (void)vector; (void)handler;
    return 0;
}
