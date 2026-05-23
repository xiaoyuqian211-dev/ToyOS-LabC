/* Minimal kernel printf and panic implementation. */
#include <stdarg.h>
#include "types.h"
#include "defs.h"

static void print_unsigned(uint64 x, uint base, int is_signed) {
  char buf[32];
  int i = 0;

  if (is_signed && (int64)x < 0) {
    console_putchar('-');
    x = (uint64)(-(int64)x);
  }

  if (x == 0) {
    console_putchar('0');
    return;
  }

  while (x > 0) {
    uint digit = x % base;
    buf[i++] = (char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
    x /= base;
  }
  while (i-- > 0) {
    console_putchar(buf[i]);
  }
}

static void vprintf(const char *fmt, va_list ap) {
  for (int i = 0; fmt[i]; i++) {
    if (fmt[i] != '%') {
      console_putchar(fmt[i]);
      continue;
    }

    char c = fmt[++i];
    if (c == 0) {
      break;
    }
    switch (c) {
      case 's': {
        const char *s = va_arg(ap, const char *);
        console_puts(s ? s : "(null)");
        break;
      }
      case 'd':
        print_unsigned((uint64)va_arg(ap, int), 10, 1);
        break;
      case 'x':
        print_unsigned((uint64)va_arg(ap, uint64), 16, 0);
        break;
      case 'p':
        console_puts("0x");
        print_unsigned((uint64)va_arg(ap, void *), 16, 0);
        break;
      case 'c':
        console_putchar(va_arg(ap, int));
        break;
      case '%':
        console_putchar('%');
        break;
      default:
        console_putchar('%');
        console_putchar(c);
        break;
    }
  }
}

void printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

void panic(const char *fmt, ...) {
  va_list ap;
  console_puts("[ToyOS] panic: ");
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  console_putchar('\n');
  for (;;) {
    asm volatile("wfi");
  }
}
