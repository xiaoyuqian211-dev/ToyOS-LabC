# ToyOS-LabC build system for RISC-V 64 on QEMU virt.

BUILD := build

CROSS ?= $(shell if command -v riscv64-unknown-elf-gcc >/dev/null 2>&1; then \
	echo riscv64-unknown-elf-; else echo riscv64-linux-gnu-; fi)
CC := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
QEMU ?= qemu-system-riscv64

COMMON_FLAGS := -march=rv64gc -mabi=lp64 -mcmodel=medany -ffreestanding \
	-fno-common -fno-builtin -fno-pic -fno-pie -nostdlib \
	-Wall -Wextra -Iinclude

CFLAGS := $(COMMON_FLAGS) -O2
ASFLAGS := $(COMMON_FLAGS)
USER_FLAGS := $(COMMON_FLAGS) -O2 -Iuser

KERNEL_C_SRCS := \
	kernel/main.c \
	kernel/console.c \
	kernel/printf.c \
	kernel/sbi.c \
	kernel/string.c \
	kernel/kalloc.c \
	kernel/vm.c \
	kernel/trap.c \
	kernel/syscall.c \
	kernel/proc.c \
	kernel/timer.c \
	kernel/fs.c

KERNEL_S_SRCS := \
	kernel/entry.S \
	kernel/trap.S \
	kernel/swtch.S

KERNEL_OBJS := \
	$(patsubst kernel/%.c,$(BUILD)/kernel/%.o,$(KERNEL_C_SRCS)) \
	$(patsubst kernel/%.S,$(BUILD)/kernel/%.o,$(KERNEL_S_SRCS)) \
	$(BUILD)/kernel/userbins.o

UPROGS := init sh hello forkdemo cat
USER_ELFS := $(addprefix $(BUILD)/user/,$(addsuffix .elf,$(UPROGS)))

KERNEL_ELF := $(BUILD)/kernel.elf
KERNEL_BIN := $(BUILD)/kernel.bin

.PHONY: all clean run test check-env

all: $(KERNEL_ELF) $(KERNEL_BIN)

$(BUILD)/kernel $(BUILD)/user:
	mkdir -p $@

$(BUILD)/kernel/%.o: kernel/%.c include/*.h | $(BUILD)/kernel
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/kernel/%.o: kernel/%.S include/*.h | $(BUILD)/kernel
	$(CC) $(ASFLAGS) -c -o $@ $<

$(BUILD)/kernel/userbins.o: kernel/userbins.S $(USER_ELFS) | $(BUILD)/kernel
	$(CC) $(ASFLAGS) -c -o $@ $<

$(BUILD)/user/%.elf: user/%.c user/syscall.c user/usys.S user/user.h user/user.ld include/syscall.h | $(BUILD)/user
	$(CC) $(USER_FLAGS) -T user/user.ld -Wl,-z,max-page-size=4096 -o $@ user/usys.S user/syscall.c $<

$(KERNEL_ELF): $(KERNEL_OBJS) kernel/linker.ld
	$(CC) $(CFLAGS) -T kernel/linker.ld -Wl,-z,max-page-size=4096 -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

run: $(KERNEL_ELF)
	$(QEMU) -machine virt -bios default -m 128M -smp 1 -nographic -kernel $(KERNEL_ELF)

test:
	sh scripts/run_tests.sh

check-env:
	sh scripts/check_env.sh

clean:
	rm -rf $(BUILD)
