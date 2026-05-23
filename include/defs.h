/* Cross-module kernel declarations. */
#ifndef TOYOS_DEFS_H
#define TOYOS_DEFS_H

#include "types.h"
#include "riscv.h"
#include "proc.h"

/* console.c */
void console_init(void);
void console_putchar(int ch);
void console_puts(const char *s);

/* printf.c */
void printf(const char *fmt, ...);
void panic(const char *fmt, ...) __attribute__((noreturn));

/* sbi.c */
void sbi_shutdown(void) __attribute__((noreturn));
void sbi_set_timer(uint64 stime);

/* string.c */
void *memset(void *dst, int c, uint64 n);
void *memmove(void *dst, const void *src, uint64 n);
void *memcpy(void *dst, const void *src, uint64 n);
int memcmp(const void *a, const void *b, uint64 n);
uint64 strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, uint64 n);
char *strncpy(char *dst, const char *src, uint64 n);

/* kalloc.c */
void kinit(void);
void *kalloc(void);
void kfree(void *pa);

/* vm.c */
void kvminit(void);
void kvminithart(void);
pagetable_t kernel_pagetable_get(void);
pte_t *walk(pagetable_t pagetable, uint64 va, int alloc);
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm);
pagetable_t uvmcreate(void);
uint64 uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int perm);
void uvmfree(pagetable_t pagetable, uint64 sz);
int uvmcopy(pagetable_t old, pagetable_t new, uint64 sz);
int copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len);
int copyout(pagetable_t pagetable, uint64 dstva, const char *src, uint64 len);
int copyinstr(pagetable_t pagetable, char *dst, uint64 srcva, uint64 max);
uint64 walkaddr(pagetable_t pagetable, uint64 va);

/* trap.c / trap.S */
void trapinit(void);
void trap_entry(void);
void trap_dispatch(struct trapframe *tf);
void trap_return(void) __attribute__((noreturn));
void userret(struct trapframe *tf, uint64 satp) __attribute__((noreturn));

/* syscall.c */
void syscall_dispatch(void);

/* proc.c / swtch.S */
void procinit(void);
void userinit(void);
void scheduler(void) __attribute__((noreturn));
void sched(void);
void proc_yield(void);
int proc_fork(void);
void proc_exit(int status) __attribute__((noreturn));
int proc_wait(uint64 status_user);
int proc_exec(const char *name);
struct proc *myproc(void);
void swtch(struct context *old, struct context *new);
void proc_dump(void);

/* timer.c */
void timer_init(void);

/* fs.c */
void fs_init(void);
int fs_read(const char *path, char *dst, int max);

#endif
