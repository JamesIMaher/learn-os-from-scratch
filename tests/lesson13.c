/* Lesson 13 checkpoints: ELF loading and processes. */
#include "harness.h"
#include "proc/process.h"
#include "proc/thread.h"
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "mm/heap.h"
#include "abi/errno.h"

static const void *image(const char *name, size_t *len)
{
    const multiboot_module_t *m = t_find_module(name);
    if (!m) { *len = 0; return 0; }
    *len = m->mod_end - m->mod_start;
    return (const void *)(uintptr_t)m->mod_start;
}

/* A modified copy of an ELF image: 32-bit `value` written at `offset`. */
static void *patched(const void *img, size_t len, uint32_t offset, uint32_t value)
{
    uint8_t *copy = kmalloc(len);
    if (!copy) return 0;
    for (size_t i = 0; i < len; i++) copy[i] = ((const uint8_t *)img)[i];
    for (int i = 0; i < 4; i++) copy[offset + i] = (uint8_t)(value >> (8 * i));
    return copy;
}

static int load_into_new(const void *img, size_t len, uintptr_t *entry)
{
    address_space_t *as = vmm_create_space();
    int r = elf_load(as, img, len, entry);
    vmm_destroy_space(as);
    return r;
}

static int spawn_and_wait(const char *name)
{
    size_t len;
    const void *img = image(name, &len);
    int pid = process_spawn_image(img, len, name);
    if (pid <= 0) return -1000 + pid;
    int status = -999;
    if (process_wait(pid, &status) != pid) return -2000;
    return status;
}

void lesson13(void)
{
    t_lesson(13, "Processes and ELF");
    size_t hlen, elen, clen;
    const void *hello = image("hello.elf", &hlen);
    const void *elfcheck = image("elfcheck.elf", &elen);
    image("crash.elf", &clen);

    t_begin("the test programs were loaded as multiboot modules");
    CHECK(hello && elfcheck && clen);
    t_end();
    if (!hello || !elfcheck || !clen) return;

    t_begin("elf_load maps a valid program and returns its entry point");
    address_space_t *as = vmm_create_space();
    uintptr_t entry = 0;
    CHECK_EQ(elf_load(as, hello, hlen, &entry), 0);
    CHECK_EQ(entry, *(const uint32_t *)((const uint8_t *)hello + 24));   /* e_entry */
    uint32_t flags = 0;
    CHECK_EQ(vmm_translate(as, entry, 0, &flags), 0);
    CHECK(flags & VMM_USER);
    vmm_destroy_space(as);
    t_end();

    t_begin("elf_load rejects things that aren't valid i386 executables");
    static const char junk[] = "this is definitely not an ELF file, not at all.......";
    CHECK_EQ(load_into_new(junk, sizeof junk, &entry), -ENOEXEC);
    CHECK_EQ(load_into_new(hello, 30, &entry), -ENOEXEC);            /* truncated */
    void *bad = patched(hello, hlen, 18, 62);                         /* e_machine = x86-64 */
    CHECK_EQ(load_into_new(bad, hlen, &entry), -ENOEXEC);
    kfree(bad);
    bad = patched(hello, hlen, 24, 0x00100000);                       /* entry in the kernel */
    CHECK_EQ(load_into_new(bad, hlen, &entry), -ENOEXEC);
    kfree(bad);
    bad = patched(hello, hlen, 28, 0x7FFFFFF0);                       /* e_phoff out of bounds */
    CHECK_EQ(load_into_new(bad, hlen, &entry), -ENOEXEC);
    kfree(bad);
    t_end();

    t_begin("hello runs as a process and its exit status comes back");
    size_t pos = 0;
    char token[64];
    int pid = process_spawn_image(hello, hlen, "hello");
    CHECK(pid > 0);
    int status = -1;
    CHECK_EQ(process_wait(pid, &status), pid);
    CHECK_EQ(status, 7);
    t_fmt_str(token, &pos, "Hello, user space! pid=");
    t_fmt_dec(token, &pos, pid);
    t_expect_serial(token);
    t_end();

    t_begin("data, bss, rodata, and the stack are set up correctly");
    CHECK_EQ(spawn_and_wait("elfcheck.elf"), 42);
    t_expect_serial("elfcheck-all-good");
    t_end();

    t_begin("a crashing process is killed; the kernel carries on");
    CHECK_EQ(spawn_and_wait("crash.elf"), EXIT_KILLED);
    t_expect_serial("crash-about-to-fault");
    t_end();

    t_begin("process_wait only works once, and only for real children");
    pid = process_spawn_image(hello, hlen, "hello");
    CHECK_EQ(process_wait(pid, &status), pid);
    CHECK_EQ(process_wait(pid, &status), -ECHILD);
    CHECK_EQ(process_wait(12345, &status), -ECHILD);
    t_end();

    t_begin("several processes at once, waited for in any order");
    int pids[6];
    for (int i = 0; i < 6; i++) pids[i] = process_spawn_image(hello, hlen, "hello");
    bool distinct = true;
    for (int i = 0; i < 6; i++)
        for (int j = 0; j < i; j++) if (pids[i] == pids[j]) distinct = false;
    CHECK(distinct);
    for (int i = 5; i >= 0; i--) {
        status = -1;
        CHECK_EQ(process_wait(pids[i], &status), pids[i]);
        CHECK_EQ(status, 7);
    }
    t_end();

    t_begin("processes don't leak memory");
    struct t_mem m0;
    t_mem_snapshot(&m0);
    for (int i = 0; i < 25; i++) {
        spawn_and_wait("elfcheck.elf");
        spawn_and_wait("crash.elf");
    }
    CHECK(t_mem_no_leak(&m0, 32768));
    t_end();
}
