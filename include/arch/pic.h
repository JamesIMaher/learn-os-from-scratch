/*
 * pic.h - the 8259 Programmable Interrupt Controller pair.     [Lesson 06]
 *
 * By default the PICs deliver IRQ 0-7 on vectors 8-15, which collide with CPU
 * exceptions. Remap them: IRQ n must arrive on vector PIC_IRQ_BASE + n.
 */
#pragma once
#include <stdint.h>
#include "arch/idt.h"

#define PIC_IRQ_BASE 32

/* Remap both PICs and mask every IRQ line (except the cascade, IRQ 2). */
void pic_init(void);

void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);

/* Tell the PIC(s) the handler for `irq` is done. Which chips need to hear it? */
void pic_send_eoi(uint8_t irq);

/*
 * Install a handler for hardware IRQ `irq` (0..15) and unmask it.
 * Your dispatcher must send the EOI for IRQs; handlers do not.
 */
void irq_register(uint8_t irq, isr_handler_t handler);
