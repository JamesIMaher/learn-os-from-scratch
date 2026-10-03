# Lesson 15: A shell

> **Goal:** a user-space shell. Type commands on the keyboard, and your
> kernel runs them.
> **Files:** `user/bin/sh.c` (and whatever kernel fixes it flushes out)
> **Checkpoint:** `make check L=15` (the harness types commands at your shell)

## Why this matters

Everything you've built meets here: the keyboard driver and its IRQ, the
scheduler blocking a reader, system calls with validated pointers, the
filesystem, ELF loading, processes and exit statuses. A shell is just a user
program, but it's the first one that *uses* your OS the way a person does.
It will find bugs in earlier lessons. That's part of the lesson.

## Required behavior

- Print the prompt `$ ` and read a line from fd 0. Echo each character as it's
  typed (the kernel doesn't echo). Backspace removes the last character
  from the line *and* from the screen. Enter ends the line.
- Split the line into words on spaces (runs of spaces count as one).
- Builtins:
  - `echo WORDS...` prints the words separated by single spaces, then a newline;
  - `ls [DIR]` lists a directory (default `/`), one name per line;
  - `cat FILE...` prints files; a missing file prints `cat: FILE: not found`;
  - `help` lists what you can do;
  - `exit` ends the shell with status 0.
- Anything else runs `/bin/WORD` (or the word itself if it starts with `/`)
  and waits for it. If its exit status isn't 0, print `[status N]`. If it
  doesn't exist, print `WORD: command not found`.

Make your kernel's `kmain` spawn `/bin/sh` at the end of boot, and wait for it.

## Questions before you code

1. The kernel's `read(0, ...)` doesn't echo. What are the pros and cons of
   echoing in the shell vs. in the kernel (a "line discipline")?
2. How do you erase a character on screen *and* over serial with a
   backspace? (Your VGA driver's `'\b'` blanks the cell; what does a
   terminal do with `'\b'`?)
3. Splitting in place: how can you turn `"echo   one two"` into an array of
   word pointers without allocating memory?
4. `spawn` returns an error. How do you tell "no such program" from other
   failures?

## Milestones

1. A loop that prints `$ `, reads a line, and prints it back.
2. Word splitting and `echo`.
3. `ls` and `cat`.
4. Running programs, statuses, errors.
5. Boot straight into your shell with `make run`.

## The checkpoint verifies

Spawns `/bin/sh` and types (through QEMU): `echo one   two` (expects
`one two`), an `echo` line edited with backspace, `ls /`, `ls /docs`,
`cat /docs/b.txt`, running `shelltest` (expects its output and `[status 3]`),
an unknown command, `cat` of a missing file, and `exit`, then checks the
shell's exit status is 0 and that each expected output appeared on serial.

## Hints

<details><summary>Hint 1: nothing happens when I type</summary>

Work backwards with prints: is the keyboard IRQ firing? Is `SYS_READ` on fd
0 reached? Is the shell blocked with interrupts disabled? (The syscall gate
cleared IF.) Is the shell's process even running? (`spawn` result?)
</details>

<details><summary>Hint 2: backspace</summary>

On a terminal, `'\b'` only moves the cursor left. The usual trick is to send
the three characters backspace, space, backspace. Only do it if the line
isn't already empty.
</details>

## Going further

- Pipes: `SYS_PIPE` and `ls | cat`. You'll need `dup2` and a kernel
  buffer with two blocked ends. A big, satisfying addition.
- Command history with the up arrow (lesson 07's going-further).
- Run a program in the background with `&`, and a `ps` command.
- `cd` and relative paths: a current directory per process.

## Reflect

You've built an operating system. Write down, for a single key press on
the way to running `/bin/hello`, every layer it passes through, from the
keyboard controller to the moment `hello` exits and the shell prints the
next prompt.

## References

- *The Unix Programming Environment*, Kernighan & Pike, chapter 3 (the shell)
- Your own lessons 07, 11, 12, 13, 14
