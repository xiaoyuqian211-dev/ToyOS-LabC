/* User-space syscall wrappers plus a tiny printf/string library. */
#include <stdarg.h>
#include "user.h"
#include "syscall.h"

static long do_syscall(long num, long a0, long a1, long a2, long a3, long a4,
                       long a5) {
  register long x0 asm("a0") = a0;
  register long x1 asm("a1") = a1;
  register long x2 asm("a2") = a2;
  register long x3 asm("a3") = a3;
  register long x4 asm("a4") = a4;
  register long x5 asm("a5") = a5;
  register long x7 asm("a7") = num;
  asm volatile("ecall"
               : "+r"(x0)
               : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x7)
               : "memory");
  return x0;
}

long write(int fd, const void *buf, int n) {
  return do_syscall(SYS_write, fd, (long)buf, n, 0, 0, 0);
}

void exit(int status) {
  (void)do_syscall(SYS_exit, status, 0, 0, 0, 0, 0);
  for (;;) {
  }
}

int getpid(void) {
  return (int)do_syscall(SYS_getpid, 0, 0, 0, 0, 0, 0);
}

int yield(void) {
  return (int)do_syscall(SYS_yield, 0, 0, 0, 0, 0, 0);
}

int fork(void) {
  return (int)do_syscall(SYS_fork, 0, 0, 0, 0, 0, 0);
}

int wait(int *status) {
  return (int)do_syscall(SYS_wait, (long)status, 0, 0, 0, 0, 0);
}

int exec(const char *name) {
  return (int)do_syscall(SYS_exec, (long)name, 0, 0, 0, 0, 0);
}

int readfile(const char *path, char *buf, int max) {
  return (int)do_syscall(SYS_readfile, (long)path, (long)buf, max, 0, 0, 0);
}

int info(void) {
  return (int)do_syscall(SYS_info, 0, 0, 0, 0, 0, 0);
}

uint64 strlen(const char *s) {
  uint64 n = 0;
  while (s[n]) {
    n++;
  }
  return n;
}

int strcmp(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }
  return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, uint64 n) {
  for (uint64 i = 0; i < n; i++) {
    if (a[i] != b[i] || a[i] == 0 || b[i] == 0) {
      return (unsigned char)a[i] - (unsigned char)b[i];
    }
  }
  return 0;
}

char *strcpy(char *dst, const char *src) {
  char *p = dst;
  while ((*p++ = *src++) != 0) {
  }
  return dst;
}

int puts(const char *s) {
  int n = (int)strlen(s);
  write(1, s, n);
  write(1, "\n", 1);
  return n + 1;
}

static void putch(char c) {
  write(1, &c, 1);
}

static void print_unsigned(uint64 x, int base, int is_signed) {
  char buf[32];
  int i = 0;
  if (is_signed && (long)x < 0) {
    putch('-');
    x = (uint64)(-(long)x);
  }
  if (x == 0) {
    putch('0');
    return;
  }
  while (x > 0) {
    uint d = (uint)(x % (uint64)base);
    buf[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
    x /= (uint64)base;
  }
  while (i-- > 0) {
    putch(buf[i]);
  }
}

void printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  for (int i = 0; fmt[i]; i++) {
    if (fmt[i] != '%') {
      putch(fmt[i]);
      continue;
    }
    char c = fmt[++i];
    switch (c) {
      case 's': {
        const char *s = va_arg(ap, const char *);
        write(1, s ? s : "(null)", (int)strlen(s ? s : "(null)"));
        break;
      }
      case 'd':
        print_unsigned((uint64)va_arg(ap, int), 10, 1);
        break;
      case 'x':
        print_unsigned((uint64)va_arg(ap, uint64), 16, 0);
        break;
      case 'p':
        write(1, "0x", 2);
        print_unsigned((uint64)va_arg(ap, void *), 16, 0);
        break;
      case 'c':
        putch((char)va_arg(ap, int));
        break;
      case '%':
        putch('%');
        break;
      default:
        putch('%');
        putch(c);
        break;
    }
  }
  va_end(ap);
}
