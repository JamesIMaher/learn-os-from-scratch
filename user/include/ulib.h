/*
 * ulib.h - the C library for user programs.              [Lessons 13-15]
 *
 * User programs can't call kernel functions: they're in a different
 * privilege level and (in principle) a different world. Everything they get
 * from the kernel goes through `int 0x80` (see include/abi/syscall_nums.h).
 *
 * TODO(lesson 13): implement user/lib/crt0.asm, user/lib/syscalls.c,
 * user/lib/ustring.c and user/lib/uprintf.c. (You've written a string
 * library and a printf before. Reuse what you learned - or your code.)
 */
#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "syscall_nums.h"
#include "errno.h"
#include "fs.h"

/* ---- system call wrappers: return what the kernel returns -------------- */
__attribute__((noreturn)) void exit(int status);
long write(int fd, const void *buf, size_t n);
long read(int fd, void *buf, size_t n);
int  yield(void);
int  getpid(void);
int  sleep_ms(unsigned ms);
int  spawn(const char *path);
int  wait(int pid, int *status);
int  open(const char *path);
int  close(int fd);
int  readdir(const char *path, unsigned index, struct vfs_dirent *out);

/* ---- strings (standard C semantics) ----------------------------------- */
void  *memset(void *dst, int value, size_t n);
void  *memcpy(void *dst, const void *src, size_t n);
int    memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strcpy(char *dst, const char *src);
char  *strchr(const char *s, int c);

/* ---- formatted output to fd 1 (same format rules as the kernel's) ----- */
int printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
int snprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int puts(const char *s);   /* writes s and a newline */

/* Every program defines this. */
int main(void);
