/* Lesson 01 checkpoints: booting into C. */
#include "harness.h"

extern char stack_bottom[] __attribute__((weak));
extern char stack_top[] __attribute__((weak));
extern char _kernel_start[], _kernel_end[];

void lesson01(void)
{
    t_lesson(1, "Booting into C");

    t_begin("kmain is reached");
    CHECK(1);   /* if you can read this line, your _start called kmain */
    t_end();

    t_begin("bootloader magic is passed as the first argument");
    CHECK_EQ(t_magic, MULTIBOOT_BOOTLOADER_MAGIC);
    t_end();

    t_begin("multiboot info pointer is passed as the second argument");
    CHECK(t_mbi != 0);
    if (t_mbi) {
        CHECK(t_mbi->flags & MULTIBOOT_INFO_MEMORY);
        CHECK(t_mbi->flags & MULTIBOOT_INFO_MEM_MAP);   /* did you ask for it? */
    }
    t_end();

    t_begin("boot.asm exports stack_bottom and stack_top");
    CHECK(stack_bottom != 0);
    CHECK(stack_top != 0);
    t_end();

    t_begin("the stack is yours and big enough");
    if (stack_bottom && stack_top) {
        volatile int local = 0;
        uintptr_t here = (uintptr_t)&local;
        CHECK((uintptr_t)stack_top > (uintptr_t)stack_bottom);
        CHECK(stack_top - stack_bottom >= 8192);
        CHECK(here > (uintptr_t)stack_bottom && here < (uintptr_t)stack_top);
        CHECK((uintptr_t)stack_bottom >= (uintptr_t)_kernel_start);
        CHECK((uintptr_t)stack_top <= (uintptr_t)_kernel_end);
        CHECK((uintptr_t)stack_top % 16 == 0);
    } else {
        CHECK(!"stack symbols missing");
    }
    t_end();

    t_begin("interrupts are still disabled");
    CHECK((t_eflags() & (1u << 9)) == 0);
    t_end();
}
