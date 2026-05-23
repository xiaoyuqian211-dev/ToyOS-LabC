/* System call dispatcher and kernel-side syscall implementations. */
#include "types.h"
#include "param.h"
#include "syscall.h"
#include "defs.h"

static uint64 argraw(int n) {
  struct trapframe *tf = myproc()->trapframe;
  switch (n) {
    case 0: return tf->a0;
    case 1: return tf->a1;
    case 2: return tf->a2;
    case 3: return tf->a3;
    case 4: return tf->a4;
    case 5: return tf->a5;
    default: return 0;
  }
}

static int sys_write(void) {
  int fd = (int)argraw(0);
  uint64 ubuf = argraw(1);
  int n = (int)argraw(2);
  if (fd != 1 && fd != 2) {
    return -1;
  }
  if (n < 0) {
    return -1;
  }
  for (int i = 0; i < n; i++) {
    char ch;
    if (copyin(myproc()->pagetable, &ch, ubuf + (uint64)i, 1) != 0) {
      return -1;
    }
    console_putchar(ch);
  }
  return n;
}

static int sys_exit(void) {
  int status = (int)argraw(0);
  proc_exit(status);
}

static int sys_getpid(void) {
  return myproc()->pid;
}

static int sys_yield(void) {
  proc_yield();
  return 0;
}

static int sys_fork(void) {
  return proc_fork();
}

static int sys_wait(void) {
  return proc_wait(argraw(0));
}

static int sys_exec(void) {
  char path[MAXPATH];
  if (copyinstr(myproc()->pagetable, path, argraw(0), sizeof(path)) != 0) {
    return -1;
  }
  return proc_exec(path);
}

static int sys_readfile(void) {
  char path[MAXPATH];
  uint64 ubuf = argraw(1);
  int max = (int)argraw(2);
  char tmp[256];

  if (max < 0) {
    return -1;
  }
  if (max > (int)sizeof(tmp)) {
    max = (int)sizeof(tmp);
  }
  if (copyinstr(myproc()->pagetable, path, argraw(0), sizeof(path)) != 0) {
    return -1;
  }
  int n = fs_read(path, tmp, max);
  if (n < 0) {
    return -1;
  }
  if (copyout(myproc()->pagetable, ubuf, tmp, (uint64)n) != 0) {
    return -1;
  }
  return n;
}

static int sys_info(void) {
  proc_dump();
  return 0;
}

void syscall_dispatch(void) {
  struct proc *p = myproc();
  int num = (int)p->trapframe->a7;
  int ret = -1;

  switch (num) {
    case SYS_write: ret = sys_write(); break;
    case SYS_exit: ret = sys_exit(); break;
    case SYS_getpid: ret = sys_getpid(); break;
    case SYS_yield: ret = sys_yield(); break;
    case SYS_fork: ret = sys_fork(); break;
    case SYS_wait: ret = sys_wait(); break;
    case SYS_exec: ret = sys_exec(); break;
    case SYS_readfile: ret = sys_readfile(); break;
    case SYS_info: ret = sys_info(); break;
    default:
      printf("[ToyOS] unknown syscall %d from pid=%d\n", num, p->pid);
      ret = -1;
      break;
  }

  p->trapframe->a0 = (uint64)ret;
}
