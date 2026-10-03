/* syscall.c - see include/proc/syscall.h and include/abi/. [Lessons 12-15] */
#include "proc/syscall.h"
#include "abi/syscall_nums.h"
#include "abi/errno.h"

/*
 * TODO(lesson 12): syscall_init, user_range_ok, and a dispatcher for
 * SYS_EXIT, SYS_WRITE, SYS_YIELD, SYS_GETPID, SYS_SLEEP. Unknown numbers
 * return -ENOSYS. Arguments and the return value travel in the interrupt
 * frame's registers.
 * TODO(lesson 13): SYS_WAIT.
 * TODO(lesson 14): SYS_READ, SYS_OPEN, SYS_CLOSE, SYS_READDIR, SYS_SPAWN.
 */

void syscall_init(void) { }
int user_range_ok(address_space_t *as, uintptr_t ptr, size_t len, int writable)
{
    (void)as; (void)ptr; (void)len; (void)writable;
    return 0;
}
