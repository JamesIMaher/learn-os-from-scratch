/*
 * idt.h - interrupts and CPU exceptions.                       [Lesson 05]
 *
 * Every interrupt (CPU exception, hardware IRQ, or software `int N`) must
 * end up calling ONE C dispatcher with a pointer to a struct interrupt_frame
 * describing the interrupted CPU state. The dispatcher calls whatever
 * handler was registered for that vector.
 *
 * Your assembly stubs must build exactly this layout on the stack. The field
 * order tells you the push order. (Why is it "backwards"?)
 *
 * If a handler modifies the frame, the modification must take effect when
 * the interrupt returns. That is how system calls return values (lesson 12)
 * and how a page-fault handler could skip an instruction.
 */
#pragma once
#include <stdint.h>

struct interrupt_frame {
    /* Pushed by your common stub, last-pushed first. */
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_unused, ebx, edx, ecx, eax;   /* PUSHAD */
    /* Pushed by your per-vector stub. */
    uint32_t vector;
    uint32_t err_code;   /* real error code for some exceptions, 0 otherwise */
    /* Pushed by the CPU itself. */
    uint32_t eip, cs, eflags;
    uint32_t user_esp, user_ss;   /* ONLY present if the interrupt came from ring 3 */
} __attribute__((packed));

typedef void (*isr_handler_t)(struct interrupt_frame *frame);

/*
 * Build and load the IDT. After this:
 *   - vectors 0..31 (CPU exceptions), 32..47 (hardware IRQs, lesson 06) and
 *     0x80 (system calls, lesson 12) all reach your C dispatcher;
 *   - vector 0x80 can be invoked with `int 0x80` from ring 3 (lesson 12);
 *   - an exception with no registered handler panics with a useful dump
 *     (vector name, error code, EIP, and the general registers).
 * Do NOT enable interrupts (sti) here.
 */
void idt_init(void);

/* Install `handler` for `vector`. Returns the previously installed handler
 * (NULL if none), so a caller can temporarily take over a vector and restore it. */
isr_handler_t isr_register(uint8_t vector, isr_handler_t handler);
