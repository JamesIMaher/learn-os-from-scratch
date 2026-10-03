; =============================================================================
; isr.asm - interrupt entry stubs.                                 [Lesson 05]
;
; TODO(lesson 05). Every vector needs a small entry point whose address goes
; into the IDT. Each one should make the stack look the same (some exceptions
; push an error code and some don't - which ones?), push its vector number,
; and jump to one common stub. The common stub saves everything else so the
; stack matches struct interrupt_frame (include/arch/idt.h), loads the kernel
; data segments, calls your C dispatcher with a pointer to the frame, then
; undoes all of it and returns with IRET.
;
; Writing 48+ nearly identical stubs by hand is error prone. NASM has
; macros (%macro / %rep) - look them up.
; =============================================================================

bits 32
section .text
