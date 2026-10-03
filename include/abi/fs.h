/* fs.h - filesystem structures shared by kernel and user programs. [Lesson 14] */
#pragma once
#include <stdint.h>

#define VFS_FILE 1
#define VFS_DIR  2

#define VFS_NAME_MAX 64   /* including the NUL */

struct vfs_stat {
    uint32_t type;      /* VFS_FILE or VFS_DIR      */
    uint32_t size;      /* bytes (0 for directories) */
};

struct vfs_dirent {
    char     name[VFS_NAME_MAX];   /* just the last path component, e.g. "a.txt" */
    uint32_t type;
    uint32_t size;
};
