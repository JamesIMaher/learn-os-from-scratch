# Lesson 08: Physical memory

> **Goal:** know which RAM exists, which is free, and hand it out one 4 KiB
> page frame at a time.
> **Files:** `src/mm/pmm.c`
> **Checkpoint:** `make check L=8`

## Why this matters

Everything that needs memory from now on (page tables, the heap, thread
stacks, user programs) ultimately gets it from this allocator. It's
deliberately simple: fixed-size frames, no sizes to remember, no
fragmentation. The interesting parts are *finding* the memory and *never*
handing out memory that's already in use.

## Background

### The memory map

The PC's physical address space is full of holes: legacy BIOS areas below
1 MiB, the VGA buffer, ACPI tables, memory-mapped devices near 4 GiB. Only
the firmware knows where usable RAM is. The bootloader asked it and passes
you a **memory map** (`mmap_addr` / `mmap_length` in the multiboot info):
a list of (start, length, type) regions. Type 1 is usable RAM. Read the
spec carefully: the entries have a `size` field and are *not* simply an
array of structs.

### Usable is not the same as free

Usable RAM already contains things you need: your kernel image, the
bootloader's info structures, and the *modules* the bootloader loaded (your
future programs and filesystem). The contract in `include/mm/pmm.h` lists
what must stay untouched. Print the addresses of everything the bootloader
gave you before you decide what to reserve. One of them may surprise you.

### Bookkeeping

You need to record, for each frame, whether it's free. Options include:

| design | alloc | free | overhead | notes |
|--------|-------|------|----------|-------|
| bitmap | search for a 0 bit | clear a bit | 1 bit per frame | simple; search can be slow |
| free list | pop | push | free frames store the "next" pointer themselves | O(1), but can't easily check double frees |
| stack of frame numbers | pop | push | 4 bytes per frame | O(1) |

Pick one and be able to say why. Where does the bookkeeping itself live?

## Your mission

Implement `include/mm/pmm.h`. In `kmain`, print the memory map (start, end,
type of each region) and the totals your allocator found.

## Questions before you code

1. Write out the memory map QEMU gives you with `-m 128M` (print it
   first!). Which regions are usable? How many frames is that?
2. How do you step from one mmap entry to the next?
3. `addr` and `len` are 64-bit. What do you do with a usable region above 4 GiB?
4. Usable regions may not start or end on a 4 KiB boundary. Should you round
   in or out? Why?
5. `extern char _kernel_end[];` vs `extern uint32_t _kernel_end;`: what does
   each one's *value* mean in C? Which gives you the address?
6. Why is physical address 0 a perfect "out of memory" value here?

## Milestones

1. The memory map prints correctly.
2. Allocate three frames and print them; free them; allocate again; you get
   them back.
3. Allocate until you get 0; the count matches what you predicted.

## The checkpoint verifies

That you found about 128 MiB (30000-32768 frames); that 2000 allocations are
distinct, page-aligned, above 1 MiB, inside RAM, and never in the kernel
image, a module, the module list or a command-line string; that writing to
every allocated frame doesn't corrupt your allocator; that the free count is
exact through alloc and free; and that you can allocate *every* free frame
and then get 0.

## Hints

<details><summary>Hint 1: walking the map</summary>

Treat `mmap_addr` as a byte pointer. Each entry starts with its `size`
field, and the next entry begins `size + 4` bytes later (the size doesn't
count itself). Stop at `mmap_addr + mmap_length`.
</details>

<details><summary>Hint 2: an easy bitmap setup</summary>

Start with every frame marked *used*. Walk the map and mark frames inside
usable regions as *free*. Then mark the reserved ranges used again. That way
holes in the map are safe by default.
</details>

<details><summary>Hint 3: a test fails on "never reserved memory"</summary>

Print `mbi->mods_addr` and each module's `cmdline` address. Under QEMU some
of the bootloader's data lives just above your kernel, inside a region the
map calls usable. Reserve every page touched by the module list and by each
string, not just the modules themselves.
</details>

<details><summary>Hint 4: allocation is slow</summary>

A bitmap scan from frame 0 every time is O(n). Remember where the last search
ended, or skip whole 32-bit words that are all ones.
</details>

## Going further

- Allocate *contiguous* runs of frames (DMA needs them).
- Add a debug mode that fills freed frames with `0xDEADBEEF` and detects
  double frees.

## Reflect

- Once paging is on (next lesson), will a pointer with the value of a frame's
  physical address still reach that frame? Under what arrangement would it?
  (This course chose that arrangement deliberately. Lesson 09 explains.)

## References

- Multiboot Specification §3.3 ("Boot information format": mmap, mods)
- OSDev wiki: "Detecting Memory (x86)", "Page Frame Allocation"
