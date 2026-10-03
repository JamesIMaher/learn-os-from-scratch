/*
 * timer.h - the 8253/8254 Programmable Interval Timer.         [Lesson 06]
 *
 * Program PIT channel 0 to fire IRQ 0 periodically and count the ticks.
 * The counter is the kernel's sense of time.
 */
#pragma once
#include <stdint.h>

/* Program the PIT to `hz` interrupts per second and install the IRQ 0 handler.
 * Does not enable interrupts by itself. */
void timer_init(uint32_t hz);

uint32_t timer_hz(void);

/* Ticks since timer_init. 64 bits so it never wraps. On a 32-bit CPU, reading
 * a 64-bit value that an interrupt handler updates has a trap in it. Find it. */
uint64_t timer_ticks(void);

/* Wait at least `ms` milliseconds. Interrupts must be enabled.
 * Don't burn the CPU in a tight loop: there's an instruction for waiting. */
void timer_sleep_ms(uint32_t ms);
