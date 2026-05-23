#!/usr/bin/env sh
# Build and run ToyOS on QEMU virt.
set -eu

make
exec qemu-system-riscv64 -machine virt -bios default -m 128M -smp 1 -nographic -kernel build/kernel.elf
