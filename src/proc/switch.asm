; =============================================================================
; switch.asm - the context switch.                                 [Lesson 11]
;
; TODO(lesson 11). A function along the lines of
;     void context_switch(uint32_t *save_esp_here, uint32_t new_esp);
; Which registers does cdecl promise are preserved across a call? Those are
; the only ones you need to save. EIP is saved for you... by whom?
; =============================================================================

bits 32
section .text
