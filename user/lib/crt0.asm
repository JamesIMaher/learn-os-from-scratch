; =============================================================================
; crt0.asm - "C runtime zero": where a user program really starts.  [Lesson 13]
;
; The kernel jumps to the ELF entry point, `_start`, with ESP at the top of
; the user stack and nothing else set up. C's main() expects to be called
; like a normal function, and when it returns, the process must exit with
; main's return value.
;
; TODO(lesson 13): write _start. Put it in section .text.start so user.ld
; places it first (handy when reading disassembly). Export it with `global`.
; =============================================================================

bits 32
section .text.start
global _start
_start:
    ud2             ; TODO(lesson 13): replace this placeholder (it just crashes)
