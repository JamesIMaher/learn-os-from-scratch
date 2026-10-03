/*
 * string.h - the kernel's tiny C library: memory and string helpers. [Lesson 02]
 *
 * You have no libc. The compiler is still allowed to emit calls to memcpy,
 * memset, memmove and memcmp on its own (e.g. for struct assignment), so
 * these four MUST exist and MUST be correct, or very strange bugs follow.
 *
 * Semantics are those of standard C. `man 3 memmove` etc. on any Linux box.
 */
#pragma once
#include <stddef.h>

void  *memset(void *dst, int value, size_t n);
void  *memcpy(void *dst, const void *src, size_t n);   /* regions must not overlap */
void  *memmove(void *dst, const void *src, size_t n);  /* regions MAY overlap      */
int    memcmp(const void *a, const void *b, size_t n);

size_t strlen(const char *s);
int    strcmp(const char *a, const char *b);
int    strncmp(const char *a, const char *b, size_t n);
char  *strcpy(char *dst, const char *src);
char  *strncpy(char *dst, const char *src, size_t n);  /* standard (odd!) semantics */
char  *strchr(const char *s, int c);
