/* fsuser - exercises the file and process system calls from user space.
 * Exits with 0 if all is well, otherwise the number of the failed step. */
#include "ulib.h"

#define STEP(n, cond) do { if (!(cond)) { printf("fsuser: step %d failed\n", n); return n; } } while (0)

int main(void)
{
    char buf[64];
    int fd = open("/hello.txt");
    STEP(1, fd == 3);                               /* lowest free fd */
    long n = read(fd, buf, sizeof buf - 1);
    STEP(2, n == 23);
    buf[n] = 0;
    STEP(3, strcmp(buf, "Hello from the initrd!\n") == 0);
    STEP(4, read(fd, buf, sizeof buf) == 0);        /* end of file */
    int fd2 = open("/docs/a.txt");
    STEP(5, fd2 == 4);
    STEP(6, close(fd) == 0);
    STEP(7, open("/docs/b.txt") == 3);              /* 3 is free again */
    STEP(8, close(fd) == 0 && close(fd) == -EBADF);
    STEP(9, read(99, buf, 1) == -EBADF);
    STEP(10, read(fd2, (void *)0x00100000, 4) == -EFAULT);
    STEP(11, open((const char *)0x00100000) == -EFAULT);
    STEP(12, open("/nope.txt") == -ENOENT);
    STEP(13, write(fd2, "x", 1) == -EBADF);         /* files are read-only */

    struct vfs_dirent d;
    int count = 0;
    while (readdir("/docs", (unsigned)count, &d) == 0) count++;
    STEP(14, count == 2);
    STEP(15, readdir("/docs", 0, (struct vfs_dirent *)0x00100000) == -EFAULT);

    int pid = spawn("/bin/hello");
    STEP(16, pid > 0);
    int status = -1;
    STEP(17, wait(pid, &status) == pid && status == 7);
    STEP(18, wait(pid, &status) == -ECHILD);
    STEP(19, spawn("/nope") == -ENOENT);
    STEP(20, spawn("/hello.txt") == -ENOEXEC);
    int pid2 = spawn("/bin/hello");
    STEP(21, wait(pid2, (int *)0x00100000) == -EFAULT);   /* pointer checked first */
    STEP(22, wait(pid2, &status) == pid2 && status == 7);  /* ...so it's still waitable */
    puts("fsuser-all-good");
    return 0;
}
