/*
 * serial.h - 16550 UART driver for COM1.                       [Lesson 02]
 *
 * Under QEMU, whatever you send to COM1 appears on your terminal
 * (`make run` passes -serial stdio). This is your lifeline: it works before
 * the screen does, it can be logged to a file, and it survives a crash.
 */
#pragma once
#include <stdbool.h>

#define COM1_PORT 0x3F8

/*
 * Configure COM1: 38400 baud (or any rate), 8 data bits, no parity, one stop
 * bit, FIFOs on. Then run the UART's loopback self-test.
 * Returns 0 on success, -1 if the self-test fails (no working UART).
 */
int  serial_init(void);

/* Send one byte, waiting until the transmitter can accept it.
 * '\n' must be sent as "\r\n" (why do terminals want that?). */
void serial_putc(char c);
void serial_write(const char *s);

/* Is there a received byte waiting? */
bool serial_can_read(void);
/* Wait for and return one received byte. */
char serial_getc(void);
