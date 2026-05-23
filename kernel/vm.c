/* Sv39 page table management for kernel and user address spaces. */
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

static pagetable_t kernel_pagetable;

pagetable_t kernel_pagetable_get(void) {
  return kernel_pagetable;
}

pte_t *walk(pagetable_t pagetable, uint64 va, int alloc) {
  if (va >= MAXVA) {
    return NULL;
  }

  for (int level = 2; level > 0; level--) {
    pte_t *pte = &pagetable[PX(level, va)];
    if (*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if (!alloc) {
        return NULL;
      }
      pagetable_t next = (pagetable_t)kalloc();
      if (next == NULL) {
        return NULL;
      }
      memset(next, 0, PGSIZE);
      *pte = PA2PTE(next) | PTE_V;
      pagetable = next;
    }
  }
  return &pagetable[PX(0, va)];
}

uint64 walkaddr(pagetable_t pagetable, uint64 va) {
  pte_t *pte = walk(pagetable, va, 0);
  if (pte == NULL || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0) {
    return 0;
  }
  return PTE2PA(*pte) + (va % PGSIZE);
}

int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm) {
  if (size == 0) {
    return -1;
  }
  uint64 a = PGROUNDDOWN(va);
  uint64 last = PGROUNDDOWN(va + size - 1);

  for (;;) {
    pte_t *pte = walk(pagetable, a, 1);
    if (pte == NULL) {
      return -1;
    }
    if (*pte & PTE_V) {
      return -1;
    }
    *pte = PA2PTE(pa) | perm | PTE_V;
    if (a == last) {
      break;
    }
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

static void map_kernel_range(pagetable_t kpgtbl, uint64 va, uint64 pa,
                             uint64 size, int perm) {
  if (mappages(kpgtbl, va, size, pa, perm) != 0) {
    panic("kernel mapping failed va=%p", (void *)va);
  }
}

void kvminit(void) {
  kernel_pagetable = (pagetable_t)kalloc();
  if (kernel_pagetable == NULL) {
    panic("kvminit out of memory");
  }
  memset(kernel_pagetable, 0, PGSIZE);

  map_kernel_range(kernel_pagetable, UART0, UART0, UART0_SIZE, PTE_R | PTE_W);
  map_kernel_range(kernel_pagetable, KERNBASE, KERNBASE, PHYSTOP - KERNBASE,
                   PTE_R | PTE_W | PTE_X);
}

void kvminithart(void) {
  w_satp(make_satp(kernel_pagetable));
  sfence_vma();
}

pagetable_t uvmcreate(void) {
  pagetable_t pagetable = (pagetable_t)kalloc();
  if (pagetable == NULL) {
    return NULL;
  }
  memset(pagetable, 0, PGSIZE);

  /*
   * User page tables do not copy root entry 0, because user low addresses live
   * there. They do copy the kernel direct-map root entries so S-mode trap code
   * can run immediately after an ecall, before switching back to kernel_satp.
   */
  for (int i = PX(2, KERNBASE); i < 512; i++) {
    pagetable[i] = kernel_pagetable[i];
  }
  return pagetable;
}

uint64 uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int perm) {
  if (newsz < oldsz) {
    return oldsz;
  }
  oldsz = PGROUNDUP(oldsz);
  for (uint64 a = oldsz; a < newsz; a += PGSIZE) {
    char *mem = (char *)kalloc();
    if (mem == NULL) {
      return 0;
    }
    if (mappages(pagetable, a, PGSIZE, (uint64)mem, perm | PTE_U) != 0) {
      kfree(mem);
      return 0;
    }
  }
  return newsz;
}

static void uvmunmap_user(pagetable_t pagetable, uint64 va, uint64 size,
                          int do_free) {
  uint64 a = PGROUNDDOWN(va);
  uint64 last = PGROUNDDOWN(va + size - 1);
  for (;;) {
    pte_t *pte = walk(pagetable, a, 0);
    if (pte && (*pte & PTE_V) && (*pte & PTE_U)) {
      if (do_free) {
        kfree((void *)PTE2PA(*pte));
      }
      *pte = 0;
    }
    if (a == last) {
      break;
    }
    a += PGSIZE;
  }
}

static void freewalk_user(pagetable_t pagetable, int level, int root) {
  int start = 0;
  int end = root ? PX(2, KERNBASE) : 512;
  for (int i = start; i < end; i++) {
    pte_t pte = pagetable[i];
    if ((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0) {
      freewalk_user((pagetable_t)PTE2PA(pte), level - 1, 0);
      pagetable[i] = 0;
    }
  }
  (void)level;
  kfree((void *)pagetable);
}

void uvmfree(pagetable_t pagetable, uint64 sz) {
  if (pagetable == NULL) {
    return;
  }
  if (sz > 0) {
    uvmunmap_user(pagetable, 0, sz, 1);
  }
  uvmunmap_user(pagetable, USER_TOP - USER_STACK_PAGES * PGSIZE,
                USER_STACK_PAGES * PGSIZE, 1);
  freewalk_user(pagetable, 2, 1);
}

int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz) {
  for (uint64 va = 0; va < sz; va += PGSIZE) {
    pte_t *pte = walk(old, va, 0);
    if (pte == NULL || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0) {
      continue;
    }
    char *mem = (char *)kalloc();
    if (mem == NULL) {
      return -1;
    }
    memmove(mem, (void *)PTE2PA(*pte), PGSIZE);
    int flags = (int)(*pte & (PTE_R | PTE_W | PTE_X | PTE_U));
    if (mappages(new, va, PGSIZE, (uint64)mem, flags) != 0) {
      kfree(mem);
      return -1;
    }
  }

  uint64 stack = USER_TOP - PGSIZE;
  pte_t *spte = walk(old, stack, 0);
  if (spte && (*spte & PTE_V) && (*spte & PTE_U)) {
    char *mem = (char *)kalloc();
    if (mem == NULL) {
      return -1;
    }
    memmove(mem, (void *)PTE2PA(*spte), PGSIZE);
    int flags = (int)(*spte & (PTE_R | PTE_W | PTE_X | PTE_U));
    if (mappages(new, stack, PGSIZE, (uint64)mem, flags) != 0) {
      kfree(mem);
      return -1;
    }
  }
  return 0;
}

int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len) {
  while (len > 0) {
    uint64 va0 = PGROUNDDOWN(srcva);
    uint64 pa0 = walkaddr(pagetable, va0);
    if (pa0 == 0) {
      return -1;
    }
    uint64 n = PGSIZE - (srcva - va0);
    if (n > len) {
      n = len;
    }
    memmove(dst, (void *)(pa0 + (srcva - va0)), n);
    len -= n;
    dst += n;
    srcva = va0 + PGSIZE;
  }
  return 0;
}

int copyout(pagetable_t pagetable, uint64 dstva, const char *src, uint64 len) {
  while (len > 0) {
    uint64 va0 = PGROUNDDOWN(dstva);
    uint64 pa0 = walkaddr(pagetable, va0);
    if (pa0 == 0) {
      return -1;
    }
    uint64 n = PGSIZE - (dstva - va0);
    if (n > len) {
      n = len;
    }
    memmove((void *)(pa0 + (dstva - va0)), src, n);
    len -= n;
    src += n;
    dstva = va0 + PGSIZE;
  }
  return 0;
}

int copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max) {
  for (uint64 i = 0; i < max; i++) {
    uint64 pa = walkaddr(pagetable, srcva + i);
    if (pa == 0) {
      return -1;
    }
    char c = *(char *)pa;
    dst[i] = c;
    if (c == 0) {
      return 0;
    }
  }
  if (max > 0) {
    dst[max - 1] = 0;
  }
  return -1;
}
