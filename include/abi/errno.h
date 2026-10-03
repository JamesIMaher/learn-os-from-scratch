/* errno.h - error numbers, shared by kernel and user programs. Functions
 * return them NEGATED, e.g. `return -EFAULT;`. */
#pragma once

#define ENOENT   2   /* no such file or directory      */
#define ENOEXEC  8   /* not a valid executable         */
#define EBADF    9   /* bad file descriptor            */
#define ECHILD  10   /* no such child process          */
#define ENOMEM  12   /* out of memory                  */
#define EFAULT  14   /* bad address                    */
#define EINVAL  22   /* invalid argument               */
#define EMFILE  24   /* too many open files            */
#define ENOSYS  38   /* no such system call            */
