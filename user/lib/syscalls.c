/* syscalls.c - system call wrappers.                            [Lesson 13] */
#include "ulib.h"

/*
 * TODO(lesson 13). Each wrapper puts its number and arguments in the right
 * registers, executes `int 0x80`, and returns EAX. One well-written inline
 * asm helper can serve all of them. Tell the compiler which registers the
 * asm reads and writes, and that it may touch memory ("memory" clobber):
 * why does that matter for write()?
 */

void exit(int status) { (void)status; for (;;) { } }
long write(int fd, const void *buf, size_t n) { (void)fd; (void)buf; (void)n; return -ENOSYS; }
long read(int fd, void *buf, size_t n) { (void)fd; (void)buf; (void)n; return -ENOSYS; }
int yield(void) { return -ENOSYS; }
int getpid(void) { return -ENOSYS; }
int sleep_ms(unsigned ms) { (void)ms; return -ENOSYS; }
int spawn(const char *path) { (void)path; return -ENOSYS; }
int wait(int pid, int *status) { (void)pid; (void)status; return -ENOSYS; }
int open(const char *path) { (void)path; return -ENOSYS; }
int close(int fd) { (void)fd; return -ENOSYS; }
int readdir(const char *path, unsigned index, struct vfs_dirent *out)
{
    (void)path; (void)index; (void)out;
    return -ENOSYS;
}
