/* timer.c - see include/drivers/timer.h.                        [Lesson 06] */
#include "drivers/timer.h"
#include "arch/io.h"
#include "arch/pic.h"

/*
 * TODO(lesson 06). The PIT's input clock runs at 1193182 Hz. Channel 0 divides
 * it by a 16-bit reload value that you choose. Ports: 0x40 (channel 0 data),
 * 0x43 (mode/command). Which operating mode gives a periodic interrupt?
 */

void timer_init(uint32_t hz) { (void)hz; }
uint32_t timer_hz(void) { return 0; }
uint64_t timer_ticks(void) { return 0; }
void timer_sleep_ms(uint32_t ms) { (void)ms; }
