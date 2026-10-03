/* Lesson 14 checkpoints: the filesystem and file system calls. */
#include "harness.h"
#include "fs/vfs.h"
#include "proc/process.h"
#include "abi/errno.h"

#define BIG_SIZE 100000u
static uint8_t big_byte(uint32_t i) { return (uint8_t)(i * 7 + i / 251); }   /* see tools/mkbig.py */

static uint8_t buf[8192];

static bool has_entry(const char *dir, const char *name, uint32_t type, int *count)
{
    struct vfs_dirent d;
    bool found = false;
    *count = 0;
    for (uint32_t i = 0; i < 100 && vfs_readdir(dir, i, &d) == 0; i++) {
        (*count)++;
        if (t_streq(d.name, name) && d.type == type) found = true;
    }
    return found;
}

void lesson14(void)
{
    t_lesson(14, "A filesystem");

    t_begin("fs_init finds the initrd");
    CHECK_EQ(fs_init(t_mbi), 0);
    t_end();

    t_begin("vfs_stat knows files and directories");
    struct vfs_stat st = { 0, 0 };
    CHECK_EQ(vfs_stat("/hello.txt", &st), 0);
    CHECK_EQ(st.type, VFS_FILE);
    CHECK_EQ(st.size, 23);
    CHECK_EQ(vfs_stat("/big.bin", &st), 0);
    CHECK_EQ(st.size, BIG_SIZE);
    CHECK_EQ(vfs_stat("/empty.txt", &st), 0);
    CHECK_EQ(st.size, 0);
    CHECK_EQ(vfs_stat("/", &st), 0);
    CHECK_EQ(st.type, VFS_DIR);
    CHECK_EQ(vfs_stat("/docs", &st), 0);
    CHECK_EQ(st.type, VFS_DIR);
    CHECK_EQ(vfs_stat("/docs/", &st), 0);
    CHECK_EQ(st.type, VFS_DIR);
    CHECK_EQ(vfs_stat("/docs/a.txt", &st), 0);
    CHECK_EQ(st.type, VFS_FILE);
    CHECK_EQ(vfs_stat("/nope", &st), -ENOENT);
    CHECK_EQ(vfs_stat("/hello.txt/x", &st), -ENOENT);
    CHECK_EQ(vfs_stat("/doc", &st), -ENOENT);         /* prefix of a real name */
    t_end();

    t_begin("reading a whole file");
    file_t *f = vfs_open("/hello.txt");
    CHECK(f != 0);
    if (f) {
        long n = vfs_read(f, buf, sizeof buf);
        CHECK_EQ(n, 23);
        buf[n > 0 ? n : 0] = 0;
        CHECK_STR((char *)buf, "Hello from the initrd!\n");
        CHECK_EQ(vfs_read(f, buf, sizeof buf), 0);
        vfs_close(f);
    }
    t_end();

    t_begin("small reads advance the offset");
    f = vfs_open("/docs/a.txt");
    CHECK(f != 0);
    if (f) {
        char out[64];
        int pos = 0;
        long n;
        while ((n = vfs_read(f, buf, 5)) > 0 && pos + n < 63)
            for (long i = 0; i < n; i++) out[pos++] = (char)buf[i];
        out[pos] = 0;
        CHECK_STR(out, "This is a.txt, inside /docs.\n");
        vfs_close(f);
    }
    t_end();

    t_begin("a 100 KB file reads back byte-for-byte");
    f = vfs_open("/big.bin");
    CHECK(f != 0);
    if (f) {
        uint32_t total = 0;
        bool same = true;
        long n;
        uint32_t chunk = 1000;
        while ((n = vfs_read(f, buf, chunk)) > 0) {
            for (long i = 0; i < n; i++) if (buf[i] != big_byte(total + (uint32_t)i)) same = false;
            total += (uint32_t)n;
            chunk = chunk == 1000 ? 4096 : 1000;
        }
        CHECK_EQ(total, BIG_SIZE);
        CHECK(same);
        vfs_close(f);
    }
    t_end();

    t_begin("empty files, missing files, and directories can't be read");
    f = vfs_open("/empty.txt");
    CHECK(f != 0);
    if (f) { CHECK_EQ(vfs_read(f, buf, 10), 0); vfs_close(f); }
    CHECK(vfs_open("/missing") == 0);
    CHECK(vfs_open("/docs") == 0);
    t_end();

    t_begin("vfs_readdir lists direct children only");
    int count;
    CHECK(has_entry("/", "hello.txt", VFS_FILE, &count));
    CHECK(has_entry("/", "docs", VFS_DIR, &count));
    CHECK(has_entry("/", "bin", VFS_DIR, &count));
    CHECK(has_entry("/", "big.bin", VFS_FILE, &count));
    CHECK(has_entry("/", "empty.txt", VFS_FILE, &count));
    CHECK_EQ(count, 5);
    CHECK(has_entry("/docs", "a.txt", VFS_FILE, &count));
    CHECK(has_entry("/docs/", "b.txt", VFS_FILE, &count));
    CHECK_EQ(count, 2);
    CHECK(has_entry("/bin", "hello", VFS_FILE, &count));
    CHECK(has_entry("/bin", "sh", VFS_FILE, &count));
    struct vfs_dirent d;
    CHECK_EQ(vfs_readdir("/hello.txt", 0, &d), -ENOENT);
    CHECK_EQ(vfs_readdir("/nope", 0, &d), -ENOENT);
    t_end();

    t_begin("process_spawn loads programs from the filesystem");
    int pid = process_spawn("/bin/hello");
    CHECK(pid > 0);
    int status = -1;
    CHECK_EQ(process_wait(pid, &status), pid);
    CHECK_EQ(status, 7);
    CHECK_EQ(process_spawn("/bin/nope"), -ENOENT);
    CHECK_EQ(process_spawn("/hello.txt"), -ENOEXEC);
    t_end();

    t_begin("user programs can open, read, readdir, spawn and wait");
    pid = process_spawn("/bin/fsuser");
    CHECK(pid > 0);
    status = -1;
    process_wait(pid, &status);
    CHECK_EQ(status, 0);       /* nonzero = the step number that failed (see tests/user/fsuser.c) */
    t_expect_serial("fsuser-all-good");
    t_end();
}
