/* pic.c - see include/arch/pic.h.                               [Lesson 06] */
#include "arch/pic.h"
#include "arch/io.h"

/*
 * TODO(lesson 06). The remap is a fixed sequence of four "initialization
 * command words" (ICW1-ICW4) sent to each chip. Find what each one means
 * rather than copying the magic numbers. Master: ports 0x20/0x21,
 * slave: 0xA0/0xA1.
 */

void pic_init(void) { }
void pic_mask(uint8_t irq) { (void)irq; }
void pic_unmask(uint8_t irq) { (void)irq; }
void pic_send_eoi(uint8_t irq) { (void)irq; }
void irq_register(uint8_t irq, isr_handler_t handler) { (void)irq; (void)handler; }
