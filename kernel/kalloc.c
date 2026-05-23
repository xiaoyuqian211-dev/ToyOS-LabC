/* Physical page allocator: a simple free-list over 4 KiB pages. */
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "defs.h"

extern char end[];

struct run {
  struct run *next;
};

static struct {
  struct run *freelist;
  uint64 free_pages;
} kmem;

static void freerange(uint64 pa_start, uint64 pa_end) {
  uint64 p = PGROUNDUP(pa_start);
  for (; p + PGSIZE <= pa_end; p += PGSIZE) {
    kfree((void *)p);
  }
}

void kinit(void) {
  kmem.freelist = NULL;
  kmem.free_pages = 0;
  freerange((uint64)end, PHYSTOP);
}

void kfree(void *pa) {
  uint64 p = (uint64)pa;
  if ((p % PGSIZE) != 0 || p < (uint64)end || p >= PHYSTOP) {
    panic("kfree invalid page %p", pa);
  }

  memset(pa, 0x5, PGSIZE);
  struct run *r = (struct run *)pa;
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.free_pages++;
}

void *kalloc(void) {
  struct run *r = kmem.freelist;
  if (r == NULL) {
    return NULL;
  }
  kmem.freelist = r->next;
  kmem.free_pages--;
  memset((void *)r, 0, PGSIZE);
  return (void *)r;
}
