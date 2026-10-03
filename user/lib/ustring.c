/* ustring.c - string functions for user programs.               [Lesson 13] */
#include "ulib.h"

/* TODO(lesson 13) */
void *memset(void *dst, int value, size_t n) { (void)value; (void)n; return dst; }
void *memcpy(void *dst, const void *src, size_t n) { (void)src; (void)n; return dst; }
int memcmp(const void *a, const void *b, size_t n) { (void)a; (void)b; (void)n; return 0; }
size_t strlen(const char *s) { (void)s; return 0; }
int strcmp(const char *a, const char *b) { (void)a; (void)b; return 0; }
int strncmp(const char *a, const char *b, size_t n) { (void)a; (void)b; (void)n; return 0; }
char *strcpy(char *dst, const char *src) { (void)src; return dst; }
char *strchr(const char *s, int c) { (void)s; (void)c; return 0; }
