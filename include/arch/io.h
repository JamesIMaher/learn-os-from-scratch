/*
 * io.h - x86 port I/O.                                        [Lesson 02]
 *
 * x86 has a second address space, separate from memory, called "I/O ports"
 * (0x0000-0xFFFF). Devices like the serial port, the PIC, the PIT and the PS/2
 * keyboard controller live there. Ordinary C cannot touch it: you need the
 * `in` and `out` instructions, which means inline assembly.
 *
 * TODO(lesson 02): implement these. Read the GCC manual section "Extended
 * Asm" and the Intel SDM entries for IN and OUT. Pay attention to which
 * registers those instructions REQUIRE their operands to be in.
 */
#pragma once
#include <stdint.h>

/* Write one byte to an I/O port. */
static inline void outb(uint16_t port, uint8_t value)
{
    (void)port; (void)value;
    /* TODO(lesson 02) */
}

/* Read one byte from an I/O port. */
static inline uint8_t inb(uint16_t port)
{
    (void)port;
    /* TODO(lesson 02) */
    return 0;
}

/*
 * Wait a tiny amount of time (roughly 1-4 microseconds) so that slow, old
 * devices (like the PIC) can keep up. Think: what harmless port could you
 * write to?
 */
static inline void io_wait(void)
{
    /* TODO(lesson 06) */
}
