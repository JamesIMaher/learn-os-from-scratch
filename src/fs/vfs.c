/* vfs.c - see include/fs/vfs.h.                                 [Lesson 14] */
#include "fs/vfs.h"
#include "abi/errno.h"

/*
 * TODO(lesson 14). A USTAR archive is a sequence of 512-byte headers, each
 * followed by the file's data padded to a multiple of 512, ended by two
 * all-zero blocks. Look at one yourself:
 *     xxd build/initrd.tar | less
 * Decide whether to walk the archive on every lookup or to build an index
 * once in fs_init. What does each choice cost?
 */

struct file {
    int todo_replace_me;   /* TODO(lesson 14) */
};

int fs_init(multiboot_info_t *mbi) { (void)mbi; return -1; }
int vfs_stat(const char *path, struct vfs_stat *out) { (void)path; (void)out; return -ENOENT; }
file_t *vfs_open(const char *path) { (void)path; return 0; }
long vfs_read(file_t *f, void *buf, size_t n) { (void)f; (void)buf; (void)n; return 0; }
void vfs_close(file_t *f) { (void)f; }
int vfs_readdir(const char *path, uint32_t index, struct vfs_dirent *out)
{
    (void)path; (void)index; (void)out;
    return -ENOENT;
}
