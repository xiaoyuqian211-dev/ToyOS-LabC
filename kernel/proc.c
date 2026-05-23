/* Process table, ELF loading, cooperative scheduling and fork/wait/exec. */
#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"

#define ELF_MAGIC 0x464C457FUL
#define PT_LOAD 1
#define PF_X 1
#define PF_W 2
#define PF_R 4

struct elfhdr {
  uchar ident[16];
  ushort type;
  ushort machine;
  uint version;
  uint64 entry;
  uint64 phoff;
  uint64 shoff;
  uint flags;
  ushort ehsize;
  ushort phentsize;
  ushort phnum;
  ushort shentsize;
  ushort shnum;
  ushort shstrndx;
};

struct proghdr {
  uint type;
  uint flags;
  uint64 off;
  uint64 vaddr;
  uint64 paddr;
  uint64 filesz;
  uint64 memsz;
  uint64 align;
};

struct user_image {
  const char *name;
  const uchar *start;
  const uchar *end;
};

extern uchar user_init_start[], user_init_end[];
extern uchar user_sh_start[], user_sh_end[];
extern uchar user_hello_start[], user_hello_end[];
extern uchar user_forkdemo_start[], user_forkdemo_end[];
extern uchar user_cat_start[], user_cat_end[];

static struct user_image images[] = {
  {"init", user_init_start, user_init_end},
  {"sh", user_sh_start, user_sh_end},
  {"hello", user_hello_start, user_hello_end},
  {"forkdemo", user_forkdemo_start, user_forkdemo_end},
  {"cat", user_cat_start, user_cat_end},
};

static struct proc ptable[NPROC];
static struct proc *current;
static struct context sched_context;
static int nextpid = 1;

static void forkret(void) __attribute__((noreturn));

struct proc *myproc(void) {
  return current;
}

static const char *state_name(enum procstate st) {
  switch (st) {
    case UNUSED: return "UNUSED";
    case USED: return "USED";
    case RUNNABLE: return "RUNNABLE";
    case RUNNING: return "RUNNING";
    case SLEEPING: return "SLEEPING";
    case ZOMBIE: return "ZOMBIE";
    default: return "?";
  }
}

void procinit(void) {
  for (int i = 0; i < NPROC; i++) {
    ptable[i].pid = 0;
    ptable[i].state = UNUSED;
    ptable[i].name[0] = 0;
    ptable[i].pagetable = NULL;
    ptable[i].kstack = NULL;
    ptable[i].trapframe = NULL;
  }
}

static void freeproc(struct proc *p) {
  if (p->pagetable) {
    uvmfree(p->pagetable, p->sz);
  }
  if (p->kstack) {
    kfree(p->kstack);
  }
  if (p->trapframe) {
    kfree(p->trapframe);
  }
  memset(p, 0, sizeof(*p));
  p->state = UNUSED;
}

static struct proc *allocproc(void) {
  for (int i = 0; i < NPROC; i++) {
    struct proc *p = &ptable[i];
    if (p->state != UNUSED) {
      continue;
    }

    p->state = USED;
    p->pid = nextpid++;
    p->kstack = (char *)kalloc();
    p->trapframe = (struct trapframe *)kalloc();
    if (p->kstack == NULL || p->trapframe == NULL) {
      freeproc(p);
      return NULL;
    }
    memset(p->trapframe, 0, sizeof(*p->trapframe));
    memset(&p->context, 0, sizeof(p->context));
    p->context.ra = (uint64)forkret;
    p->context.sp = (uint64)p->kstack + PGSIZE;
    p->exit_status = 0;
    strncpy(p->name, "proc", sizeof(p->name));
    return p;
  }
  return NULL;
}

static const struct user_image *find_image(const char *name) {
  if (name[0] == '/') {
    name++;
  }
  for (uint i = 0; i < sizeof(images) / sizeof(images[0]); i++) {
    if (strcmp(name, images[i].name) == 0) {
      return &images[i];
    }
  }
  return NULL;
}

static int load_segment(pagetable_t pagetable, const uchar *elf, uint64 elf_size,
                        const struct proghdr *ph, uint64 *sz) {
  if (ph->memsz < ph->filesz) {
    return -1;
  }
  if (ph->vaddr + ph->memsz >= USER_TOP - PGSIZE) {
    return -1;
  }
  if (ph->off + ph->filesz > elf_size) {
    return -1;
  }

  int perm = PTE_U;
  if (ph->flags & PF_R) {
    perm |= PTE_R;
  }
  if (ph->flags & PF_W) {
    perm |= PTE_W;
  }
  if (ph->flags & PF_X) {
    perm |= PTE_X;
  }
  if ((perm & (PTE_R | PTE_W | PTE_X)) == 0) {
    perm |= PTE_R;
  }

  uint64 end = ph->vaddr + ph->memsz;
  uint64 newsz = uvmalloc(pagetable, *sz, end, perm & ~PTE_U);
  if (newsz == 0) {
    return -1;
  }
  *sz = newsz;
  if (copyout(pagetable, ph->vaddr, (const char *)(elf + ph->off), ph->filesz) != 0) {
    return -1;
  }
  return 0;
}

static int build_user_image(const char *name, pagetable_t *outpt,
                            uint64 *outsz, uint64 *entry, uint64 *ustack) {
  const struct user_image *img = find_image(name);
  if (img == NULL) {
    return -1;
  }
  const uchar *elf = img->start;
  uint64 elf_size = (uint64)(img->end - img->start);
  if (elf_size < sizeof(struct elfhdr)) {
    return -1;
  }

  const struct elfhdr *eh = (const struct elfhdr *)elf;
  uint magic = (uint)eh->ident[0] | ((uint)eh->ident[1] << 8) |
               ((uint)eh->ident[2] << 16) | ((uint)eh->ident[3] << 24);
  if (magic != ELF_MAGIC || eh->phentsize != sizeof(struct proghdr)) {
    return -1;
  }

  pagetable_t pagetable = uvmcreate();
  if (pagetable == NULL) {
    return -1;
  }

  uint64 sz = 0;
  for (ushort i = 0; i < eh->phnum; i++) {
    uint64 off = eh->phoff + (uint64)i * sizeof(struct proghdr);
    if (off + sizeof(struct proghdr) > elf_size) {
      uvmfree(pagetable, sz);
      return -1;
    }
    const struct proghdr *ph = (const struct proghdr *)(elf + off);
    if (ph->type != PT_LOAD) {
      continue;
    }
    if (load_segment(pagetable, elf, elf_size, ph, &sz) != 0) {
      uvmfree(pagetable, sz);
      return -1;
    }
  }

  char *stack = (char *)kalloc();
  if (stack == NULL) {
    uvmfree(pagetable, sz);
    return -1;
  }
  if (mappages(pagetable, USER_TOP - PGSIZE, PGSIZE, (uint64)stack,
               PTE_R | PTE_W | PTE_U) != 0) {
    kfree(stack);
    uvmfree(pagetable, sz);
    return -1;
  }

  *outpt = pagetable;
  *outsz = PGROUNDUP(sz);
  *entry = eh->entry;
  *ustack = USER_TOP;
  return 0;
}

void userinit(void) {
  struct proc *p = allocproc();
  if (p == NULL) {
    panic("cannot allocate init process");
  }

  uint64 entry = 0;
  uint64 sp = 0;
  if (build_user_image("init", &p->pagetable, &p->sz, &entry, &sp) != 0) {
    panic("cannot load init user image");
  }
  p->trapframe->epc = entry;
  p->trapframe->sp = sp;
  p->trapframe->status = 0;
  p->trapframe->kernel_sp = (uint64)p->kstack + PGSIZE;
  p->trapframe->kernel_satp = make_satp(kernel_pagetable_get());
  strncpy(p->name, "init", sizeof(p->name));
  p->state = RUNNABLE;
}

int proc_exec(const char *name) {
  struct proc *p = myproc();
  pagetable_t pagetable = NULL;
  uint64 sz = 0;
  uint64 entry = 0;
  uint64 sp = 0;
  if (build_user_image(name, &pagetable, &sz, &entry, &sp) != 0) {
    return -1;
  }

  pagetable_t oldpt = p->pagetable;
  uint64 oldsz = p->sz;
  p->pagetable = pagetable;
  p->sz = sz;
  memset(p->trapframe, 0, sizeof(*p->trapframe));
  p->trapframe->epc = entry;
  p->trapframe->sp = sp;
  p->trapframe->status = 0;
  p->trapframe->kernel_sp = (uint64)p->kstack + PGSIZE;
  p->trapframe->kernel_satp = make_satp(kernel_pagetable_get());
  strncpy(p->name, name[0] == '/' ? name + 1 : name, sizeof(p->name));
  if (oldpt) {
    uvmfree(oldpt, oldsz);
  }
  return 0;
}

int proc_fork(void) {
  struct proc *p = myproc();
  struct proc *np = allocproc();
  if (np == NULL) {
    return -1;
  }
  np->pagetable = uvmcreate();
  if (np->pagetable == NULL) {
    freeproc(np);
    return -1;
  }
  np->sz = p->sz;
  if (uvmcopy(p->pagetable, np->pagetable, p->sz) != 0) {
    freeproc(np);
    return -1;
  }
  memmove(np->trapframe, p->trapframe, sizeof(*p->trapframe));
  np->trapframe->a0 = 0;
  np->trapframe->kernel_sp = (uint64)np->kstack + PGSIZE;
  np->trapframe->kernel_satp = make_satp(kernel_pagetable_get());
  np->parent = p;
  strncpy(np->name, p->name, sizeof(np->name));
  np->state = RUNNABLE;
  return np->pid;
}

void sched(void) {
  struct proc *p = myproc();
  if (p == NULL) {
    panic("sched without process");
  }
  swtch(&p->context, &sched_context);
}

void proc_yield(void) {
  struct proc *p = myproc();
  if (p == NULL) {
    return;
  }
  p->state = RUNNABLE;
  sched();
}

void proc_exit(int status) {
  struct proc *p = myproc();
  if (p == NULL) {
    panic("exit without process");
  }

  if (p->pid == 1) {
    printf("[ToyOS] all basic tests finished\n");
    sbi_shutdown();
  }

  for (int i = 0; i < NPROC; i++) {
    if (ptable[i].parent == p) {
      ptable[i].parent = NULL;
    }
  }
  p->exit_status = status;
  p->state = ZOMBIE;
  sched();
  panic("zombie process resumed");
}

int proc_wait(uint64 status_user) {
  struct proc *p = myproc();
  for (;;) {
    int have_child = 0;
    for (int i = 0; i < NPROC; i++) {
      struct proc *child = &ptable[i];
      if (child->parent != p) {
        continue;
      }
      have_child = 1;
      if (child->state == ZOMBIE) {
        int pid = child->pid;
        int status = child->exit_status;
        if (status_user != 0) {
          (void)copyout(p->pagetable, status_user, (const char *)&status,
                        sizeof(status));
        }
        freeproc(child);
        return pid;
      }
    }
    if (!have_child) {
      return -1;
    }
    proc_yield();
  }
}

void proc_dump(void) {
  printf("[ToyOS] process table:\n");
  for (int i = 0; i < NPROC; i++) {
    if (ptable[i].state != UNUSED) {
      printf("  pid=%d state=%s name=%s parent=%d\n",
             ptable[i].pid, state_name(ptable[i].state), ptable[i].name,
             ptable[i].parent ? ptable[i].parent->pid : 0);
    }
  }
}

static void forkret(void) {
  trap_return();
}

void scheduler(void) {
  for (;;) {
    int found = 0;
    for (int i = 0; i < NPROC; i++) {
      struct proc *p = &ptable[i];
      if (p->state != RUNNABLE) {
        continue;
      }
      found = 1;
      current = p;
      p->state = RUNNING;
      swtch(&sched_context, &p->context);
      current = NULL;
    }
    if (!found) {
      asm volatile("wfi");
    }
  }
}
