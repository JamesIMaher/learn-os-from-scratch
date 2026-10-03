# Lesson 14: A filesystem

> **Goal:** a read-only filesystem from a tar archive the bootloader loads,
> file descriptors for processes, and system calls to open, read, list and
> spawn programs by path.
> **Files:** `src/fs/vfs.c`, `process_spawn` in `src/proc/process.c`, and
> `SYS_OPEN`, `SYS_READ`, `SYS_CLOSE`, `SYS_READDIR`, `SYS_SPAWN` in `src/proc/syscall.c`
> **Checkpoint:** `make check L=14`

## Why this matters

Files are the abstraction that makes an OS feel like one: names instead of
addresses, and "open/read/close" as one interface for very different things
(a keyboard, a file, later a pipe or a socket). You'll also add the first
per-process kernel *resource* table: file descriptors.

## Background

### The initrd

`make` builds `build/initrd.tar` from `tests/initrd/` plus every user program
(as `/bin/<name>`), and QEMU loads it as a multiboot module. A real disk
driver is a big project (see lesson 16); a tar file in memory lets you focus
on the filesystem itself.

### USTAR

A tar archive is a sequence of 512-byte header blocks, each followed by the
file's contents padded to a 512-byte boundary, ending with two all-zero
blocks. A header holds the name, the size *in octal ASCII*, and a type flag
(regular file, directory, ...). Look at the real bytes:

```sh
make initrd
tar -tvf build/initrd.tar
xxd build/initrd.tar | head -40
```

Note how names are stored (`./docs/a.txt`, `./docs/`). Your paths look like
`/docs/a.txt`. Something has to translate.

### A tiny VFS

The interface in `include/fs/vfs.h` is deliberately small: stat, open, read
(with an offset that advances), close, and list a directory by index. A
`file_t` is an open file: *which* file, plus *where you are in it*. Two opens
of the same file are independent.

### File descriptors

User programs refer to open files by small integers. Each process has a
table mapping fd → `file_t*`. 0, 1 and 2 are special (keyboard and console).
`SYS_OPEN` returns the lowest free fd ≥ 3. The table is per process: fd 3 in
one process has nothing to do with fd 3 in another.

## Your mission

1. `fs_init` and the VFS functions.
2. `process_spawn(path)`.
3. The file system calls, including `SYS_READ` on fd 0 (keyboard: block until
   at least one character, then return what's available), and copying path
   strings safely out of user memory.
4. Close a process's open files when it's cleaned up.

## Questions before you code

1. Decode a tar header's size field by hand from the `xxd` output for
   `hello.txt`. Where does the next header start?
2. Walk the archive on every lookup, or index it once at `fs_init`? What does
   each choice cost?
3. How do you decide whether `/docs` is a directory? What if an archive has
   `docs/a.txt` but no separate entry for `docs/`?
4. A user passes a path string. `user_range_ok` needs a length, but you don't
   know the length until you've found the NUL. How do you copy it safely?
5. `readdir("/", i)`: which tar entries are direct children of `/`? Of `/docs`?
6. `read(0, buf, 100)`: why return as soon as *some* characters are available
   rather than waiting for 100?

## Milestones

1. List every entry in the archive with its size from `kmain`.
2. `vfs_stat` and `vfs_open`/`vfs_read` for `/hello.txt`.
3. `readdir`, then the system calls, then `process_spawn("/bin/hello")`.

## The checkpoint verifies

`stat` on files, directories, trailing slashes, missing paths, a path through a
file, and a prefix of a real name; whole-file and 5-byte reads; a 100 KB file
read back byte-for-byte in mixed chunk sizes; empty files; opening missing
files and directories fails; `readdir` lists exactly the direct children with
types; `process_spawn` by path (and `-ENOENT`/`-ENOEXEC`); and a user program
(`tests/user/fsuser.c`) exercising every file and process system call,
including fd numbering, `-EBADF`, `-EFAULT` on bad pointers, and spawn/wait
from user space. If it fails, its exit status is the step that failed.

## Hints

<details><summary>Hint 1: normalizing names</summary>

Normalize both sides to one canonical form, for example no leading `./` or
`/`, no trailing `/`, so `/docs/`, `/docs` and `./docs/` all become `docs`,
and the root is the empty string. Then lookup is a string compare, and "is a
direct child of D" is "starts with `D/` and has no further `/`".
</details>

<details><summary>Hint 2: copying user strings</summary>

Copy one byte at a time into a fixed kernel buffer, checking each byte's
address (or each new page) with `user_range_ok`, until you copy the NUL or
the buffer fills up (then fail with `-EINVAL`). After copying, the kernel
only uses its own copy. Why does that matter if another thread could change
the user's memory?
</details>

<details><summary>Hint 3: blocking read on the keyboard</summary>

Your `keyboard_read` from lesson 07 waits with `hlt`. Called from a system
call, is the interrupt flag set? (What kind of gate is `0x80`?) For a better
design, have readers block on a wait queue that the keyboard IRQ wakes, like
your mutex.
</details>

## Going further

- A writable in-memory filesystem (ramfs): create, write, delete.
- Mount points: make `/dev/console` a file whose reads and writes go to devices.
- An ATA PIO disk driver and a FAT12/16 or ext2 reader. A great, big project.

## Reflect

- Your VFS interface knows nothing about tar. What would you need to change
  to add a second filesystem type? Sketch the struct of function pointers.
- Why do Unix systems make the keyboard and the console *files*?

## References

- POSIX "ustar Interchange Format" (search: pax ustar header)
- OSDev wiki: "USTAR", "VFS", "Initrd"
- *Operating Systems: Three Easy Pieces*: "Files and Directories"
