/* Trap classification and return-to-user preparation. */
#include "types.h"
#include "riscv.h"
#include "defs.h"

void trapinit(void) {
  w_stvec((uint64)trap_entry);
}

void trap_dispatch(struct trapframe *tf) {
  struct proc *p = myproc();
  if (p == NULL) {
    panic("trap without current process");
  }

  uint64 scause = r_scause();
  uint64 code = scause & ~SCAUSE_INTERRUPT;

  if ((scause & SCAUSE_INTERRUPT) == 0 && code == 8) {
    tf->epc += 4;
    syscall_dispatch();
    return;
  }

  if ((scause & SCAUSE_INTERRUPT) && code == 5) {
    printf("[ToyOS] timer interrupt pid=%d\n", p->pid);
    proc_yield();
    return;
  }

  if ((scause & SCAUSE_INTERRUPT) == 0) {
    if (code == 2) {
      printf("[ToyOS] illegal instruction in pid=%d epc=%p stval=%p\n",
             p->pid, (void *)tf->epc, (void *)r_stval());
    } else if (code == 12 || code == 13 || code == 15) {
      printf("[ToyOS] page fault in pid=%d scause=%x epc=%p stval=%p\n",
             p->pid, scause, (void *)tf->epc, (void *)r_stval());
    } else {
      printf("[ToyOS] unexpected trap pid=%d scause=%x epc=%p stval=%p\n",
             p->pid, scause, (void *)tf->epc, (void *)r_stval());
    }
    proc_exit(-1);
  }

  printf("[ToyOS] unhandled interrupt scause=%x\n", scause);
}

void trap_return(void) {
  struct proc *p = myproc();
  if (p == NULL || p->trapframe == NULL) {
    panic("trap_return without process");
  }

  w_stvec((uint64)trap_entry);

  uint64 x = p->trapframe->status;
  x &= ~SSTATUS_SPP;
  x |= SSTATUS_SPIE;
  p->trapframe->status = x;
  p->trapframe->kernel_sp = (uint64)p->kstack + PGSIZE;
  p->trapframe->kernel_satp = make_satp(kernel_pagetable_get());

  userret(p->trapframe, make_satp(p->pagetable));
}
