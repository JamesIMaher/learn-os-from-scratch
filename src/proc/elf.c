/* elf.c - see include/proc/process.h.                           [Lesson 13] */
#include "proc/process.h"
#include "abi/errno.h"

/*
 * TODO(lesson 13). Read the ELF header and program header table formats
 * (`man 5 elf` on Linux, or the System V ABI i386 supplement). Write your own
 * structs. Then look at a real binary:  readelf -lh build/user/bin/hello.elf
 *
 * Never trust the file: every offset and size in it must be checked against
 * `len` and against the user-space bounds before you use it.
 */

int elf_load(address_space_t *as, const void *image, size_t len, uintptr_t *entry_out)
{
    (void)as; (void)image; (void)len; (void)entry_out;
    return -ENOEXEC;
}
