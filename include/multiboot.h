/*
 * multiboot.h - Multiboot 1 structures (a direct transcription of the spec).
 *
 * Spec: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 *
 * This file is provided because copying a table from a spec teaches nothing.
 * Reading the spec to learn what each field MEANS teaches a lot: do that.
 */
#pragma once
#include <stdint.h>

/* The value the bootloader leaves in EAX when it jumps to your kernel. */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/* Bits in multiboot_info.flags: a field is only valid if its bit is set. */
#define MULTIBOOT_INFO_MEMORY   (1u << 0)  /* mem_lower / mem_upper  */
#define MULTIBOOT_INFO_CMDLINE  (1u << 2)  /* cmdline                */
#define MULTIBOOT_INFO_MODS     (1u << 3)  /* mods_count / mods_addr */
#define MULTIBOOT_INFO_MEM_MAP  (1u << 6)  /* mmap_length / mmap_addr */

typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;       /* KiB of memory below 1 MiB          */
    uint32_t mem_upper;       /* KiB of memory above 1 MiB          */
    uint32_t boot_device;
    uint32_t cmdline;         /* physical address of a C string     */
    uint32_t mods_count;
    uint32_t mods_addr;       /* physical address of multiboot_module_t[] */
    uint32_t syms[4];
    uint32_t mmap_length;     /* size in BYTES of the memory map buffer */
    uint32_t mmap_addr;       /* physical address of the first entry    */
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} __attribute__((packed)) multiboot_info_t;

#define MULTIBOOT_MEMORY_AVAILABLE 1

/*
 * One memory map entry. CAREFUL: `size` does not include itself, and entries
 * are not necessarily sizeof(multiboot_mmap_entry_t) apart. Read the spec.
 */
typedef struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

typedef struct multiboot_module {
    uint32_t mod_start;       /* physical address of the first byte  */
    uint32_t mod_end;         /* physical address one past the last  */
    uint32_t cmdline;         /* physical address of a C string      */
    uint32_t reserved;
} __attribute__((packed)) multiboot_module_t;
