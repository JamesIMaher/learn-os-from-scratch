/* serial.c - see include/drivers/serial.h.                      [Lesson 02] */
#include "drivers/serial.h"
#include "arch/io.h"

/*
 * TODO(lesson 02). The 16550 UART has eight registers at COM1_PORT+0..+7.
 * Several share an offset and are selected by the DLAB bit. Find a register
 * table (OSDev wiki "Serial Ports", or any 16550 datasheet) and work out:
 *   - how to set the baud rate divisor,
 *   - how to put the chip into loopback mode to test it,
 *   - which status bit says "you may send now" and which says "data waiting".
 */

int serial_init(void) { return -1; }
void serial_putc(char c) { (void)c; }
void serial_write(const char *s) { (void)s; }
bool serial_can_read(void) { return false; }
char serial_getc(void) { return 0; }
