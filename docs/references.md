# References

## The primary sources

- **Intel® 64 and IA-32 Architectures Software Developer's Manuals (SDM)**.
  Free from Intel. Vol. 2 is the instruction reference (look up any
  instruction). Vol. 3A is the system programming guide: segmentation
  (ch. 3), paging (ch. 4), protection (ch. 5), interrupts (ch. 6), tasks/TSS
  (ch. 7). This is the authority; tutorials are not.
- **Multiboot Specification 0.6.96** (GNU): what the bootloader does and
  gives you.
- **System V ABI, Intel386 Architecture Processor Supplement**: calling
  convention, stack alignment, ELF details.
- **Device datasheets**: 16550 UART, 8259A PIC, 8254 PIT, 8042 keyboard
  controller. Search for the part numbers.
- **POSIX ustar format** (in the `pax` specification).

## Explanations

- **OSDev wiki** (wiki.osdev.org): excellent concept pages on every topic
  here. Many pages include complete code. Read the explanation, close the
  tab, then write yours.
- **Operating Systems: Three Easy Pieces**, Remzi & Andrea Arpaci-Dusseau.
  Free online. The best general OS textbook for the *ideas*.
- **xv6 book** (MIT 6.1810): a small Unix in ~10k lines. Save the source for
  after you've written yours.

## Tools

- GCC manual: "Extended Asm", "Machine Constraints", "Code Gen Options"
- NASM manual
- GNU ld manual: "Linker Scripts"
- QEMU documentation: "Invocation", "QEMU Monitor"
- `readelf`, `objdump`, `nm`, `addr2line`, `xxd`: learn them, they'll answer
  questions faster than any search.
