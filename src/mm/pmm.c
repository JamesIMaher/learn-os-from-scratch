/* pmm.c - see include/mm/pmm.h.                                 [Lesson 08] */
#include "mm/pmm.h"

/*
 * TODO(lesson 08). Decide on a data structure first: a bitmap (one bit per
 * frame), a free-list threaded through the free frames themselves, a stack
 * of frame numbers... Each has a different cost for init, alloc, free and
 * memory overhead. Pick one you can defend.
 *
 * The linker script defines _kernel_start and _kernel_end. They are symbols,
 * not variables: declare them as `extern char _kernel_start[];` and use their
 * ADDRESS. Why would `extern uint32_t _kernel_start;` be a trap?
 */

void pmm_init(multiboot_info_t *mbi) { (void)mbi; }
uintptr_t pmm_alloc_frame(void) { return 0; }
void pmm_free_frame(uintptr_t frame) { (void)frame; }
size_t pmm_free_count(void) { return 0; }
size_t pmm_total_count(void) { return 0; }
