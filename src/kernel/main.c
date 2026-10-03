/*
 * main.c - your kernel's C entry point.
 *
 * boot.asm calls kmain(). This is YOUR playground: as you finish each
 * lesson, initialize the new subsystem here and try it out. Order matters,
 * because subsystems depend on one another. Part of every lesson is figuring
 * out where the new init call belongs.
 *
 * (The checkpoint tests don't use this kmain; they bring the system up
 * themselves. So experiment freely.)
 */
#include <stdint.h>
#include "multiboot.h"

void kmain(uint32_t magic, multiboot_info_t *mbi)
{
    (void)magic;
    (void)mbi;

    /*
     * TODO(lesson 01): prove you got here. You have no printf and no
     * drivers yet, but the screen's text buffer is plain memory at physical
     * address 0xB8000. Two bytes per character cell. What happens if you
     * write to it?
     */

    /* TODO(lessons 02-15): initialize subsystems here, in a sensible order. */

    for (;;) {
        /* TODO: what should the CPU do when there's nothing to do? */
    }
}
