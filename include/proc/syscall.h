/*
 * syscall.h - the kernel side of system calls.          [Lessons 12-15]
 *
 * The user/kernel contract (numbers, registers, error codes) is in
 * include/abi/. This header is the kernel's half.
 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "mm/vmm.h"

/* Install the `int 0x80` handler. */
void syscall_init(void);

/*
 * Is [ptr, ptr+len) entirely inside user space AND mapped with VMM_USER (and
 * VMM_WRITE if `writable`) in address space `as`? Every pointer a user program
 * hands the kernel must pass this check before the kernel touches it.
 * A zero-length range is always OK (nothing will be touched).
 * Watch out for ranges that wrap around the top of the address space.
 */
int  user_range_ok(address_space_t *as, uintptr_t ptr, size_t len, int writable);
