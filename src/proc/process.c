/* process.c - see include/proc/process.h.               [Lessons 13 and 14] */
#include "proc/process.h"
#include "abi/errno.h"

/*
 * TODO(lesson 13). Who frees a process's address space, and when? The
 * process can't do it while it's running inside it.
 */

int process_spawn_image(const void *image, size_t len, const char *name)
{
    (void)image; (void)len; (void)name;
    return -ENOSYS;
}

int process_wait(int pid, int *status_out)
{
    (void)pid; (void)status_out;
    return -ECHILD;
}

/* TODO(lesson 14) */
int process_spawn(const char *path)
{
    (void)path;
    return -ENOSYS;
}
