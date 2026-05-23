/* Physical and virtual memory layout for qemu-system-riscv64 virt. */
#ifndef TOYOS_MEMLAYOUT_H
#define TOYOS_MEMLAYOUT_H

#include "types.h"

#define UART0       0x10000000ULL
#define UART0_SIZE  0x100ULL

#define KERNBASE    0x80200000ULL
#define PHYSTOP     0x88000000ULL

/* Sv39 reserves the top half for sign-extended addresses. ToyOS uses low VAs. */
#define MAXVA       (1ULL << (9 + 9 + 9 + 12 - 1))

#endif
