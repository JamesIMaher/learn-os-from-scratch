/*
 * syscall_nums.h - THE user/kernel contract.            [Lessons 12-15]
 *
 * Shared by the kernel and by user programs.
 *
 * Calling convention:  `int 0x80`
 *   EAX = system call number,  EBX, ECX, EDX, ESI, EDI = arguments 1..5
 *   EAX on return = result (>= 0 success, negative errno on failure)
 *   every other register is preserved.
 *
 * Every pointer argument is checked BEFORE the call does anything else: if it
 * isn't a valid user address the call fails with -EFAULT and has no effect.
 * (SYS_WAIT's status pointer may be NULL, meaning "don't store the status".)
 */
#pragma once

/*            number        arguments                              lesson  */
#define SYS_EXIT     0  /* (int status)               no return       12 */
#define SYS_WRITE    1  /* (int fd, const void *buf, size_t n) -> n   12 */
#define SYS_READ     2  /* (int fd, void *buf, size_t n) -> n         14 */
#define SYS_YIELD    3  /* () -> 0                                    12 */
#define SYS_GETPID   4  /* () -> pid (thread id before lesson 13)     12 */
#define SYS_SLEEP    5  /* (unsigned ms) -> 0                         12 */
#define SYS_SPAWN    6  /* (const char *path) -> pid                  14 */
#define SYS_WAIT     7  /* (int pid, int *status) -> pid              13 */
#define SYS_OPEN     8  /* (const char *path) -> fd                   14 */
#define SYS_CLOSE    9  /* (int fd) -> 0                              14 */
#define SYS_READDIR 10  /* (const char *path, unsigned idx, struct vfs_dirent *) -> 0  14 */

/*
 * File descriptors: 0 = keyboard (read only), 1 and 2 = console (write only).
 * Files opened with SYS_OPEN get the lowest free fd >= 3. Files are read-only.
 * Reading fd 0 blocks until at least one character is typed, then returns
 * the characters available (up to n). It does NOT echo them.
 */
