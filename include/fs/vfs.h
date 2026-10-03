/*
 * vfs.h - a read-only filesystem backed by the initrd.         [Lesson 14]
 *
 * The initrd is a multiboot module in USTAR (tar) format whose command line
 * ends with "initrd.tar". You parse it in place: no copying needed.
 *
 * Paths are absolute and '/'-separated: "/hello.txt", "/docs/a.txt",
 * "/bin/hello". "/" is the root directory. A trailing '/' on a directory
 * ("/docs/") is accepted. Note that tar stores names like "./docs/a.txt".
 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"
#include "abi/fs.h"

typedef struct file file_t;   /* an open file: which node + current offset */

/* Find and index the initrd. Returns 0, or -1 if there is no initrd module. */
int     fs_init(multiboot_info_t *mbi);

int     vfs_stat(const char *path, struct vfs_stat *out);      /* 0 or -ENOENT */
file_t *vfs_open(const char *path);         /* NULL if missing or a directory */
/* Read up to n bytes from the current offset and advance. 0 at end of file. */
long    vfs_read(file_t *f, void *buf, size_t n);
void    vfs_close(file_t *f);

/* Fill *out with the `index`-th entry (0-based, any stable order) of directory
 * `path`. Returns 0, -ENOENT past the last entry or if `path` isn't a
 * directory. Entries are direct children only: "/" lists "docs", not "docs/a.txt". */
int     vfs_readdir(const char *path, uint32_t index, struct vfs_dirent *out);
