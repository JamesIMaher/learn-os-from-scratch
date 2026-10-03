/* vmm.c - see include/mm/vmm.h.                                 [Lesson 09] */
#include "mm/vmm.h"
#include "mm/pmm.h"

/*
 * TODO(lesson 09). Before code, draw it: a 32-bit virtual address splits into
 * three fields. What indexes the page directory, what indexes a page table,
 * what's left over? How much memory does one page table cover? How many page
 * tables does it take to cover KERNEL_SPACE_END, and how will you keep the
 * kernel half identical in every address space?
 */

struct address_space {
    int todo_replace_me;   /* TODO(lesson 09) */
};

void vmm_init(void) { }
address_space_t *vmm_kernel_space(void) { return 0; }
address_space_t *vmm_current(void) { return 0; }
address_space_t *vmm_create_space(void) { return 0; }
void vmm_destroy_space(address_space_t *as) { (void)as; }
void vmm_switch(address_space_t *as) { (void)as; }
int vmm_map(address_space_t *as, uintptr_t virt, uintptr_t phys, uint32_t flags)
{
    (void)as; (void)virt; (void)phys; (void)flags;
    return -1;
}
void vmm_unmap(address_space_t *as, uintptr_t virt) { (void)as; (void)virt; }
int vmm_translate(address_space_t *as, uintptr_t virt, uintptr_t *phys_out, uint32_t *flags_out)
{
    (void)as; (void)virt; (void)phys_out; (void)flags_out;
    return -1;
}
