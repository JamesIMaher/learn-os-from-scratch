# =============================================================================
#  learn-os-from-scratch build
#
#  You should not need to edit this file to complete the course, but you are
#  encouraged to read it. Every flag here exists for a reason; lesson 00 asks
#  you to figure out several of them.
#
#    make            build build/kernel.elf (your kernel)
#    make run        boot your kernel in QEMU (serial output in this terminal)
#    make check L=N  boot the *test* kernel and run checkpoints for lessons 1..N
#    make debug      boot your kernel paused, waiting for gdb (see docs/debugging.md)
#    make debug-check L=N   same, but for the test kernel
#    make clean
# =============================================================================

# ---- toolchain ---------------------------------------------------------------
# Prefer a real cross compiler if you have one (required on macOS).
ifneq ($(shell command -v i686-elf-gcc 2>/dev/null),)
  CC := i686-elf-gcc
  LD := i686-elf-ld
else
  CC := gcc
  LD := ld
endif
AS     := nasm
QEMU   ?= qemu-system-i386
PYTHON ?= python3

# Why each of these? Lesson 00 asks. (Hint: you have no OS underneath you.)
CFLAGS := -m32 -std=gnu11 -O2 -g \
          -ffreestanding -fno-builtin -nostdlib \
          -fno-pic -fno-pie -fno-stack-protector -fno-omit-frame-pointer \
          -mno-sse -mno-sse2 -mno-mmx -mno-80387 \
          -Wall -Wextra -Wno-unused-parameter -Wno-unused-function \
          -Iinclude
ASFLAGS := -f elf32 -g -F dwarf
LDFLAGS := -m elf_i386 -nostdlib $(shell $(LD) --help 2>/dev/null | grep -q no-warn-execstack && echo --no-warn-execstack)
LIBGCC  := $(shell $(CC) -m32 -print-libgcc-file-name)

BUILD := build

# ---- kernel ------------------------------------------------------------------
KERNEL_C   := $(shell find src -name '*.c' | sort)
KERNEL_ASM := $(shell find src -name '*.asm' | sort)
KERNEL_OBJ := $(patsubst %,$(BUILD)/kernel/%.o,$(KERNEL_ASM) $(KERNEL_C))

# The test kernel = your code + tests/. Your kmain() is renamed so the test
# runner can provide its own entry point; nothing else about your code changes.
TEST_C     := $(shell find tests -maxdepth 1 -name '*.c' | sort)
TEST_ASM   := $(shell find tests -maxdepth 1 -name '*.asm' | sort)
TEST_OBJ   := $(patsubst %,$(BUILD)/test/%.o,$(KERNEL_ASM) $(KERNEL_C) $(TEST_ASM) $(TEST_C))

DEPS := $(shell find $(BUILD) -name '*.d' 2>/dev/null)
-include $(DEPS)

.PHONY: all run debug check debug-check clean user initrd
all: $(BUILD)/kernel.elf

$(BUILD)/kernel.elf: $(KERNEL_OBJ) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(KERNEL_OBJ) $(LIBGCC)

$(BUILD)/test/kernel.elf: $(TEST_OBJ) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(TEST_OBJ) $(LIBGCC)

$(BUILD)/kernel/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD)/kernel/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -Iinclude/ $< -o $@

$(BUILD)/test/src/kernel/main.c.o: EXTRA_CFLAGS := -Dkmain=student_kmain
$(BUILD)/test/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -DTEST_BUILD -MMD -MP -c $< -o $@

$(BUILD)/test/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -Iinclude/ $< -o $@

# ---- user space (lessons 13+) -----------------------------------------------
UCFLAGS := $(filter-out -Iinclude,$(CFLAGS)) -Iuser/include -Iinclude/abi
ULIB_C   := $(shell find user/lib -name '*.c' | sort)
ULIB_ASM := $(shell find user/lib -name '*.asm' | sort)
ULIB_OBJ := $(patsubst %,$(BUILD)/user/%.o,$(ULIB_ASM) $(ULIB_C))
# Every .c in user/bin and tests/user becomes one program: build/user/<name>.elf
UPROG_SRC := $(sort $(wildcard user/bin/*.c) $(wildcard tests/user/*.c))
UPROGS    := $(patsubst %.c,$(BUILD)/user/bin/%.elf,$(notdir $(UPROG_SRC)))

user: $(UPROGS)

$(BUILD)/user/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(UCFLAGS) -MMD -MP -c $< -o $@
$(BUILD)/user/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

define UPROG_RULE
$(BUILD)/user/bin/$(basename $(notdir $(1))).elf: $(BUILD)/user/$(1).o $(ULIB_OBJ) user/user.ld
	@mkdir -p $$(dir $$@)
	$(LD) $(LDFLAGS) -T user/user.ld -o $$@ $(ULIB_OBJ) $(BUILD)/user/$(1).o $(LIBGCC)
endef
$(foreach p,$(UPROG_SRC),$(eval $(call UPROG_RULE,$(p))))

# ---- initrd (lesson 14) ------------------------------------------------------
# A plain USTAR archive. Look inside it yourself:  tar -tvf build/initrd.tar
initrd: $(BUILD)/initrd.tar
$(BUILD)/initrd.tar: $(UPROGS) $(shell find tests/initrd -type f) tools/mkbig.py
	rm -rf $(BUILD)/initrd && mkdir -p $(BUILD)/initrd/bin
	cp -r tests/initrd/. $(BUILD)/initrd/
	$(PYTHON) tools/mkbig.py $(BUILD)/initrd/big.bin
	cp $(UPROGS) $(BUILD)/initrd/bin/
	cd $(BUILD)/initrd && for f in bin/*.elf; do mv "$$f" "$${f%.elf}"; done
	tar --format=ustar --owner=0 --group=0 -cf $@ -C $(BUILD)/initrd .

# Lesson 13 loads programs straight from multiboot modules (no filesystem yet).
MODULES := $(BUILD)/initrd.tar,$(BUILD)/user/bin/hello.elf,$(BUILD)/user/bin/elfcheck.elf,$(BUILD)/user/bin/crash.elf

# Extra QEMU flags, e.g.  make check L=5 QEMU_EXTRA="-d int,cpu_reset"
QEMU_EXTRA ?=
QEMU_COMMON := -m 128M -no-reboot -initrd "$(MODULES)" $(QEMU_EXTRA)

run: $(BUILD)/kernel.elf $(BUILD)/initrd.tar
	$(QEMU) $(QEMU_COMMON) -kernel $< -serial stdio

debug: $(BUILD)/kernel.elf $(BUILD)/initrd.tar
	@echo "QEMU is paused. In another terminal:  gdb -x tools/gdbinit build/kernel.elf"
	$(QEMU) $(QEMU_COMMON) -kernel $< -serial stdio -s -S

check: $(BUILD)/test/kernel.elf $(BUILD)/initrd.tar
	@test -n "$(L)" || (echo "usage: make check L=<lesson number>"; exit 2)
	$(PYTHON) tools/check.py --qemu $(QEMU) --kernel $< --lesson $(L) -- $(QEMU_COMMON)

debug-check: $(BUILD)/test/kernel.elf $(BUILD)/initrd.tar
	@test -n "$(L)" || (echo "usage: make debug-check L=<lesson number>"; exit 2)
	@echo "QEMU is paused. In another terminal:  gdb -x tools/gdbinit build/test/kernel.elf"
	$(QEMU) $(QEMU_COMMON) -kernel $< -append "lesson=$(L)" -serial stdio \
	    -debugcon file:$(BUILD)/debugcon.log -device isa-debug-exit,iobase=0xf4,iosize=0x04 -s -S

clean:
	rm -rf $(BUILD)
