/* Small RISC-V CSR and page-table helper layer. */
#ifndef TOYOS_RISCV_H
#define TOYOS_RISCV_H

#include "types.h"
#include "param.h"
#include "memlayout.h"

typedef uint64 pte_t;
typedef uint64 *pagetable_t;

#define PTE_V (1ULL << 0)
#define PTE_R (1ULL << 1)
#define PTE_W (1ULL << 2)
#define PTE_X (1ULL << 3)
#define PTE_U (1ULL << 4)

#define SATP_SV39 (8ULL << 60)

#define PGROUNDUP(sz)   (((sz) + PGSIZE - 1) & ~(PGSIZE - 1))
#define PGROUNDDOWN(a)  ((a) & ~(PGSIZE - 1))

#define PTE2PA(pte) (((pte) >> 10) << 12)
#define PA2PTE(pa)  ((((uint64)(pa)) >> 12) << 10)

#define PXSHIFT(level) (12 + (9 * (level)))
#define PX(level, va) ((((uint64)(va)) >> PXSHIFT(level)) & 0x1FF)

#define SSTATUS_SPP  (1ULL << 8)
#define SSTATUS_SPIE (1ULL << 5)

#define SCAUSE_INTERRUPT (1ULL << 63)

static inline uint64 r_sstatus(void) {
  uint64 x;
  asm volatile("csrr %0, sstatus" : "=r"(x));
  return x;
}

static inline void w_sstatus(uint64 x) {
  asm volatile("csrw sstatus, %0" : : "r"(x));
}

static inline uint64 r_scause(void) {
  uint64 x;
  asm volatile("csrr %0, scause" : "=r"(x));
  return x;
}

static inline uint64 r_stval(void) {
  uint64 x;
  asm volatile("csrr %0, stval" : "=r"(x));
  return x;
}

static inline uint64 r_sepc(void) {
  uint64 x;
  asm volatile("csrr %0, sepc" : "=r"(x));
  return x;
}

static inline void w_sepc(uint64 x) {
  asm volatile("csrw sepc, %0" : : "r"(x));
}

static inline void w_stvec(uint64 x) {
  asm volatile("csrw stvec, %0" : : "r"(x));
}

static inline uint64 r_satp(void) {
  uint64 x;
  asm volatile("csrr %0, satp" : "=r"(x));
  return x;
}

static inline void w_satp(uint64 x) {
  asm volatile("csrw satp, %0" : : "r"(x));
}

static inline void sfence_vma(void) {
  asm volatile("sfence.vma zero, zero");
}

static inline uint64 make_satp(pagetable_t pagetable) {
  return SATP_SV39 | (((uint64)pagetable) >> 12);
}

#endif
