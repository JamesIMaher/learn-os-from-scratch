/*
 * process.h - processes and program loading.            [Lessons 13 and 14]
 *
 * A process = an address space + a main user thread + a pid + (lesson 14)
 * a table of open files. Every process has a parent (the process that spawned
 * it, or "the kernel") and only its parent may wait for it.
 *
 * User programs are ELF32 executables linked by user/user.ld. Their stack is
 * USER_STACK_PAGES pages ending at USER_STACK_TOP.
 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "mm/vmm.h"

#define USER_STACK_TOP   USER_SPACE_END
#define USER_STACK_PAGES 16

/*
 * Load an ELF image into `as`: validate the header, then for every PT_LOAD
 * segment map fresh user pages and copy the file bytes, zeroing the rest
 * (that's how .bss works). Returns 0 and the entry point, or -ENOEXEC for a
 * file that isn't a valid i386 executable (including one whose entry point
 * or segments fall outside user space), or -ENOMEM.
 */
int elf_load(address_space_t *as, const void *image, size_t len, uintptr_t *entry_out);

/* Create a process running the ELF `image` (the bytes are copied, so the
 * caller may free them afterwards). Returns its pid (> 0) or a negative errno. */
int process_spawn_image(const void *image, size_t len, const char *name);

/* Wait for child `pid` to exit; store its exit status. Returns pid, or
 * -ECHILD if `pid` is not an un-waited child of the caller. Frees everything
 * the process owned. */
int process_wait(int pid, int *status_out);

/* ---- Lesson 14 ---------------------------------------------------------- */

/* Like process_spawn_image, but loads the program from the filesystem. */
int process_spawn(const char *path);
