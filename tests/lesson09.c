/* Lesson 09 checkpoints: paging and address spaces. */
#include "harness.h"
#include "mm/vmm.h"
#include "mm/pmm.h"
#include "arch/idt.h"

static volatile uint32_t kernel_global = 0xC0FFEE;

static volatile uint32_t pf_addr, pf_err;
static volatile int pf_hits;
static uintptr_t pf_fix_frame;
static void h_pagefault(struct interrupt_frame *f)
{
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    pf_addr = cr2;
    pf_err = f->err_code;
    pf_hits++;
    /* demand paging: map the page and let the instruction run again */
    vmm_map(vmm_current(), cr2, pf_fix_frame, VMM_WRITE);
}

#define V1 0x1F000000u     /* kernel space, above RAM, below the heap */
#define V2 0x1E400000u     /* a 4 MiB region nobody has touched yet */
#define U1 0x40000000u
#define U2 0x7FFFF000u

void lesson09(void)
{
    t_lesson(9, "Paging and address spaces");

    t_begin("vmm_init turns paging on");
    vmm_init();
    CHECK(t_cr0() & 0x80000000u);
    CHECK(t_cr3() != 0);
    CHECK(t_cr3() % PAGE_SIZE == 0);
    CHECK(vmm_kernel_space() != 0);
    CHECK(vmm_current() == vmm_kernel_space());
    CHECK_EQ(kernel_global, 0xC0FFEE);      /* we're still alive */
    t_end();

    address_space_t *k = vmm_kernel_space();

    t_begin("all RAM is identity mapped, supervisor-only, writable");
    uintptr_t phys = 0;
    uint32_t flags = 0xFF;
    CHECK_EQ(vmm_translate(k, (uintptr_t)&kernel_global, &phys, &flags), 0);
    CHECK_EQ(phys, (uintptr_t)&kernel_global);
    CHECK_EQ(flags, VMM_WRITE);
    CHECK_EQ(vmm_translate(k, 0xB8123, &phys, 0), 0);
    CHECK_EQ(phys, 0xB8123);
    CHECK_EQ(vmm_translate(k, 0x07FF0004, &phys, 0), 0);
    CHECK_EQ(phys, 0x07FF0004);
    CHECK_EQ(vmm_translate(k, U1, &phys, 0), -1);
    CHECK_EQ(vmm_translate(k, V1, 0, 0), -1);
    t_end();

    t_begin("vmm_map makes a frame visible at a new virtual address");
    uintptr_t f1 = pmm_alloc_frame(), f2 = pmm_alloc_frame();
    *(volatile uint32_t *)(f1 + 16) = 0x11111111;
    *(volatile uint32_t *)(f2 + 16) = 0x22222222;
    CHECK_EQ(vmm_map(k, V1, f1, VMM_WRITE), 0);
    CHECK_EQ(*(volatile uint32_t *)(V1 + 16), 0x11111111);
    *(volatile uint32_t *)(V1 + 20) = 0xABCD;
    CHECK_EQ(*(volatile uint32_t *)(f1 + 20), 0xABCD);
    CHECK_EQ(vmm_translate(k, V1 + 0x123, &phys, &flags), 0);
    CHECK_EQ(phys, f1 + 0x123);
    CHECK_EQ(flags, VMM_WRITE);
    t_end();

    t_begin("remapping a page takes effect immediately (stale TLB?)");
    CHECK_EQ(vmm_map(k, V1, f2, VMM_WRITE), 0);
    CHECK_EQ(*(volatile uint32_t *)(V1 + 16), 0x22222222);
    t_end();

    t_begin("page faults report the address and the reason");
    vmm_unmap(k, V1);
    CHECK_EQ(vmm_translate(k, V1, 0, 0), -1);
    isr_handler_t old = isr_register(14, h_pagefault);
    pf_hits = 0;
    pf_fix_frame = f1;
    *(volatile uint32_t *)(V1 + 8) = 0x5EED;   /* faults once, then succeeds */
    CHECK_EQ(pf_hits, 1);
    CHECK_EQ(pf_addr, V1 + 8);
    CHECK_EQ(pf_err & 7, 2);                    /* not-present, write, kernel */
    CHECK_EQ(*(volatile uint32_t *)(f1 + 8), 0x5EED);
    isr_register(14, old);
    vmm_unmap(k, V1);
    t_end();

    t_begin("kernel mappings are shared by every address space");
    address_space_t *a = vmm_create_space();
    CHECK(a != 0);
    if (!a) { t_end(); return; }
    CHECK_EQ(vmm_map(k, V2, f2, VMM_WRITE), 0);  /* mapped AFTER a was created */
    vmm_switch(a);
    CHECK(vmm_current() == a);
    CHECK_EQ(kernel_global, 0xC0FFEE);
    CHECK_EQ(*(volatile uint32_t *)(V2 + 16), 0x22222222);
    vmm_switch(k);
    vmm_unmap(k, V2);
    t_end();

    t_begin("user space is private to each address space");
    address_space_t *b = vmm_create_space();
    CHECK(b != 0);
    if (!b) { t_end(); return; }
    uintptr_t p1 = pmm_alloc_frame(), p2 = pmm_alloc_frame();
    *(volatile uint32_t *)p1 = 0xAAAA0001;
    *(volatile uint32_t *)p2 = 0xBBBB0002;
    CHECK_EQ(vmm_map(a, U1, p1, VMM_USER | VMM_WRITE), 0);
    CHECK_EQ(vmm_map(b, U1, p2, VMM_USER), 0);
    CHECK_EQ(vmm_translate(a, U1 + 4, &phys, &flags), 0);   /* not the current space */
    CHECK_EQ(phys, p1 + 4);
    CHECK_EQ(flags, VMM_USER | VMM_WRITE);
    CHECK_EQ(vmm_translate(b, U1, 0, &flags), 0);
    CHECK_EQ(flags, VMM_USER);
    CHECK_EQ(vmm_translate(k, U1, 0, 0), -1);
    vmm_switch(a);
    CHECK_EQ(*(volatile uint32_t *)U1, 0xAAAA0001);
    vmm_switch(b);
    CHECK_EQ(*(volatile uint32_t *)U1, 0xBBBB0002);
    vmm_switch(k);
    t_end();

    t_begin("vmm_destroy_space frees page tables and user frames");
    vmm_destroy_space(a);
    vmm_destroy_space(b);
    size_t before = pmm_free_count();
    address_space_t *c = vmm_create_space();
    uintptr_t q1 = pmm_alloc_frame(), q2 = pmm_alloc_frame(), q3 = pmm_alloc_frame();
    CHECK_EQ(vmm_map(c, U1, q1, VMM_USER), 0);
    CHECK_EQ(vmm_map(c, U1 + PAGE_SIZE, q2, VMM_USER), 0);
    CHECK_EQ(vmm_map(c, U2, q3, VMM_USER | VMM_WRITE), 0);   /* a different page table */
    vmm_destroy_space(c);
    CHECK_EQ(pmm_free_count(), before);
    t_end();

    t_begin("many address spaces can come and go without leaking");
    before = pmm_free_count();
    for (int i = 0; i < 200; i++) {
        address_space_t *s = vmm_create_space();
        if (!s) { CHECK(s != 0); break; }
        vmm_map(s, U1 + (uint32_t)i * 0x400000u, pmm_alloc_frame(), VMM_USER);
        vmm_destroy_space(s);
    }
    CHECK_EQ(pmm_free_count(), before);
    pmm_free_frame(f1);
    pmm_free_frame(f2);
    t_end();
}
