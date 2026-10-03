/* Lesson 08 checkpoints: the physical memory manager. */
#include "harness.h"
#include "mm/pmm.h"

extern char _kernel_start[], _kernel_end[];

#define MAX_FRAMES (256u * 1024 * 1024 / PAGE_SIZE)
static uint32_t frames[MAX_FRAMES];
static uint8_t seen[MAX_FRAMES / 8];

static bool overlaps(uintptr_t f, uintptr_t start, uintptr_t end)
{
    return f + PAGE_SIZE > start && f < end;
}

static bool overlaps_string(uintptr_t f, uint32_t s)
{
    return s && overlaps(f, s, s + t_strlen((const char *)(uintptr_t)s) + 1);
}

/* modules, the module list, and the command lines */
static bool overlaps_boot_info(uintptr_t f)
{
    if ((t_mbi->flags & MULTIBOOT_INFO_CMDLINE) && overlaps_string(f, t_mbi->cmdline)) return true;
    if (!(t_mbi->flags & MULTIBOOT_INFO_MODS)) return false;
    const multiboot_module_t *m = (const multiboot_module_t *)(uintptr_t)t_mbi->mods_addr;
    if (overlaps(f, t_mbi->mods_addr, t_mbi->mods_addr + t_mbi->mods_count * sizeof *m)) return true;
    for (uint32_t i = 0; i < t_mbi->mods_count; i++)
        if (overlaps(f, m[i].mod_start, m[i].mod_end) || overlaps_string(f, m[i].cmdline)) return true;
    return false;
}

/* Is this a frame we could legitimately be handed? */
static bool frame_ok(uintptr_t f)
{
    if (f == 0 || f % PAGE_SIZE) return false;
    if (f < 0x100000) return false;
    if (f + PAGE_SIZE > (uintptr_t)_kernel_start && f < (uintptr_t)_kernel_end) return false;
    if (overlaps_boot_info(f)) return false;
    if (f >= 0x100000 + (uintptr_t)t_mbi->mem_upper * 1024) return false;
    return true;
}

static void clear_seen(void) { for (size_t i = 0; i < sizeof seen; i++) seen[i] = 0; }
static bool mark_seen(uintptr_t f)   /* false if already seen */
{
    uint32_t n = f / PAGE_SIZE;
    if (n >= MAX_FRAMES) return false;
    if (seen[n / 8] & (1 << (n % 8))) return false;
    seen[n / 8] |= (uint8_t)(1 << (n % 8));
    return true;
}

void lesson08(void)
{
    t_lesson(8, "Physical memory");
    if (!t_mbi) { t_begin("multiboot info available"); CHECK(t_mbi != 0); t_end(); return; }

    t_begin("pmm_init finds roughly 128 MiB of usable RAM");
    pmm_init(t_mbi);
    size_t total = pmm_total_count(), free0 = pmm_free_count();
    CHECK(total >= 30000);       /* 128 MiB is 32768 frames, minus holes */
    CHECK(total <= 32768);
    CHECK(free0 <= total);
    CHECK(free0 + 300 >= total); /* the kernel is small; the rest is free */
    t_end();

    t_begin("2000 allocations: distinct, aligned, and never reserved memory");
    clear_seen();
    bool all_ok = true, distinct = true;
    for (int i = 0; i < 2000; i++) {
        uintptr_t f = pmm_alloc_frame();
        frames[i] = f;
        if (!frame_ok(f)) {
            if (all_ok) { t_puts("@@NOTE bad frame: "); t_puthex(f); t_putc('\n'); }
            all_ok = false;
            continue;
        }
        if (!mark_seen(f)) distinct = false;
        /* paging is off, so the frame is reachable at its physical address */
        ((volatile uint32_t *)f)[0] = (uint32_t)i;
        ((volatile uint32_t *)f)[1023] = ~(uint32_t)i;
    }
    CHECK(all_ok);
    CHECK(distinct);
    CHECK_EQ(pmm_free_count(), free0 - 2000);
    t_end();

    t_begin("allocated frames don't overlap the allocator's own data");
    bool intact = true;
    for (int i = 0; i < 2000 && all_ok; i++) {
        volatile uint32_t *p = (volatile uint32_t *)frames[i];
        if (p[0] != (uint32_t)i || p[1023] != ~(uint32_t)i) intact = false;
    }
    CHECK(intact);
    t_end();

    t_begin("freeing gives the frames back");
    for (int i = 0; i < 2000; i++) if (frames[i]) pmm_free_frame(frames[i]);
    CHECK_EQ(pmm_free_count(), free0);
    uintptr_t f = pmm_alloc_frame();
    CHECK(frame_ok(f));
    pmm_free_frame(f);
    CHECK_EQ(pmm_free_count(), free0);
    t_end();

    t_begin("all of memory can be allocated, then 0 means 'out of memory'");
    clear_seen();
    size_t n = 0;
    bool ok = true;
    while (n < MAX_FRAMES) {
        uintptr_t g = pmm_alloc_frame();
        if (!g) break;
        if (!frame_ok(g) || !mark_seen(g)) {
            if (ok) { t_puts("@@NOTE bad or repeated frame: "); t_puthex(g); t_putc('\n'); }
            ok = false;
        }
        frames[n++] = (uint32_t)g;
    }
    CHECK(ok);
    CHECK_EQ(n, free0);
    CHECK_EQ(pmm_free_count(), 0);
    CHECK_EQ(pmm_alloc_frame(), 0);
    CHECK_EQ(pmm_alloc_frame(), 0);
    for (size_t i = 0; i < n; i++) pmm_free_frame(frames[i]);
    CHECK_EQ(pmm_free_count(), free0);
    t_end();
}
