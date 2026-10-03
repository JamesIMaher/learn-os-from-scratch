# Lesson 09: Paging and address spaces

> **Goal:** turn on virtual memory. Every address the CPU uses goes through
> page tables you control, and you can create separate address spaces that
> share the kernel but have private user halves.
> **Files:** `src/mm/vmm.c`
> **Checkpoint:** `make check L=9`

## Why this matters

Paging is what makes processes possible. It gives each program the
illusion of its own memory, stops programs from touching each other or the
kernel, lets you put memory anywhere you want, and lets the kernel find out
(through page faults) whenever a program touches something it shouldn't.
It's the single most important idea in this course. Take your time.

## Background

### Two-level paging on i386

With paging on, every 32-bit linear address is split into three fields.
The top field indexes the **page directory** (one 4 KiB page of 1024
entries, located by `CR3`), whose entry points to a **page table** (another
1024 entries), whose entry points to a 4 KiB physical **frame**. The bottom
field is the offset within that frame. Each entry also carries permission
bits: present, writable, user-accessible, and more.

Work out the split and how much memory one page table covers. Everything
else follows.

### Turning it on

Load `CR3` with the physical address of a page directory, then set the PG
bit in `CR0`. The very next instruction is fetched *through your page
tables*. So the code that enables paging had better be mapped at the same
address it's running at.

### This course's layout: identity-mapped kernel

The kernel identity-maps all RAM (virtual address = physical address) in
the low 1 GiB, supervisor-only. That means:

- your kernel keeps working, unchanged, after paging turns on;
- any frame from `pmm_alloc_frame()` can be accessed through a pointer
  with the same value, which makes editing page tables easy.

User space is `0x40000000`-`0xC0000000`, private to each address space. The
kernel half must be **identical in every address space**, including for
mappings added *later* (the heap grows at runtime). Think about how two page
directories can share page tables. Constants are in `include/mm/vmm.h`.

### The TLB

The CPU caches translations in the **Translation Lookaside Buffer**. When
you change a mapping that might be cached, you must tell the CPU, or it will
keep using the old translation. Find the instruction that invalidates a
single page, and the cheap-but-heavy way that flushes everything.

### Page faults

Touch an unmapped page (or violate a permission) and the CPU raises #PF
(vector 14) with the faulting address in a control register and an error
code describing what went wrong. Right now a kernel page fault is a bug, so
your handler should panic with a precise message.

## Your mission

Implement `include/mm/vmm.h`: build the kernel space, enable paging, the
#PF handler, create/destroy/switch address spaces, map/unmap/translate.
Call `vmm_init()` from `kmain` (after which allocator?).

## Questions before you code

1. Split `0x40123ABC` into directory index, table index and offset.
2. How much does one page table map? How many page tables does the kernel
   region (0 to `KERNEL_SPACE_END`) need? How much memory is that?
3. Which bits in a page directory entry / page table entry mean present,
   writable, user? If the PDE says "user" but the PTE doesn't, can ring 3
   access the page?
4. Two address spaces both need the kernel half. If you copy the kernel's
   page directory *entries* into each new directory when it's created, what
   happens when the heap later maps a page in a region where no page table
   existed yet? How do you avoid that?
5. Which register holds the faulting address? What do bits 0, 1 and 2 of
   the #PF error code mean?
6. `vmm_destroy_space` frees user frames and page tables. Which page tables
   must it *never* free?

## Milestones

1. Paging is on and the kernel still prints. (If not: you'll triple fault on
   the instruction right after setting CR0.PG. See debugging tips.)
2. Map a fresh frame at `0x1F000000`, write through it, read it via its
   physical address.
3. A deliberate `*(int *)0x1F000000 = 1` with nothing mapped gives a clear
   panic: address, "not present", "write", "kernel".
4. Two address spaces with different frames at the same user address.

## The checkpoint verifies

Paging on; identity mapping with the right flags; `vmm_translate` with
offsets and flags, including on a space that isn't current; mapping, writing
and reading back; that remapping takes effect immediately (catches a missing
TLB flush); a page fault reporting the right address and error code, and
resuming after the handler maps the page (demand paging!); kernel mappings
added later visible in an earlier-created space; private user mappings;
and no leaked frames after creating and destroying 200 address spaces.

## Hints

<details><summary>Hint 1: triple fault when enabling paging</summary>

Before setting PG: is your page directory's address page-aligned and
physical? Is the region containing the currently executing code, the stack,
and the page tables themselves mapped (identity)? Is the *present* bit set
on the directory entries as well as the table entries? In the QEMU monitor,
`info tlb` and `info mem` show your mappings once paging is on.
</details>

<details><summary>Hint 2: sharing the kernel half</summary>

If every page table covering kernel space is allocated *once*, up front, in
`vmm_init`, then every directory can simply copy the same kernel PDEs and no
kernel PDE ever changes afterwards. Mappings added later go into existing
(shared) page tables, so every address space sees them.
</details>

<details><summary>Hint 3: the TLB test fails</summary>

After writing a PTE that may already be cached, execute `invlpg` on that
virtual address. In GCC inline asm: an `"m"` or `"r"` operand naming the
address; check the operand form the instruction wants.
</details>

<details><summary>Hint 4: vmm_translate on a non-current space</summary>

Because RAM is identity mapped, you can read *any* page directory or page
table by its physical address, whichever CR3 is loaded. Don't go through
`CR3`; go through the `address_space_t` you were given.
</details>

## Debugging when stuck

- QEMU monitor: `info mem` (mapped ranges), `info tlb` (every mapping),
  `xp /4wx 0xADDR` (read *physical* memory), `x /4wx 0xADDR` (virtual).
- `-d int` in `build/qemu.log` shows each page fault with `CR2`.

## Going further

- **Higher-half kernel**: link the kernel at `0xC0100000` and map it there.
  What has to happen in `boot.asm` *before* C runs?
- Use 4 MiB pages (PSE) for the identity map. How much page-table memory
  does that save?
- Recursive mapping: point the last PDE at the directory itself. What
  does that make visible at the top 4 MiB?

## Reflect

- With paging on, what *physical* memory does a user program have access to?
  Who decides?
- The demand-paging checkpoint mapped a page from inside the fault handler and
  the faulting instruction then succeeded. What OS features could you build
  on that trick? (Think: lazy allocation, swapping, copy-on-write.)

## References

- Intel SDM Vol. 3A, Chapter 4: §4.3 (32-bit paging), §4.6 (access rights), §4.7 (#PF error code), §4.10 (TLBs)
- Intel SDM Vol. 2: `INVLPG`, `MOV` to/from control registers
- OSDev wiki: "Paging" (concepts)
