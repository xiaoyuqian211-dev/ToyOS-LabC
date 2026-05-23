/* Process, trapframe and context definitions for the cooperative scheduler. */
#ifndef TOYOS_PROC_H
#define TOYOS_PROC_H

#include "types.h"
#include "param.h"
#include "riscv.h"

struct trapframe {
  uint64 ra;
  uint64 gp;
  uint64 tp;
  uint64 t0;
  uint64 t1;
  uint64 t2;
  uint64 s0;
  uint64 s1;
  uint64 a0;
  uint64 a1;
  uint64 a2;
  uint64 a3;
  uint64 a4;
  uint64 a5;
  uint64 a6;
  uint64 a7;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
  uint64 t3;
  uint64 t4;
  uint64 t5;
  uint64 t6;
  uint64 sp;
  uint64 kernel_sp;
  uint64 epc;
  uint64 status;
  uint64 kernel_satp;
};

struct context {
  uint64 ra;
  uint64 sp;
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

enum procstate {
  UNUSED,
  USED,
  RUNNABLE,
  RUNNING,
  SLEEPING,
  ZOMBIE
};

struct proc {
  int pid;
  enum procstate state;
  char name[PROC_NAME];
  pagetable_t pagetable;
  uint64 sz;
  char *kstack;
  struct trapframe *trapframe;
  struct context context;
  struct proc *parent;
  int exit_status;
};

#endif
