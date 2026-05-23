/* UART console driver for the QEMU virt 16550-compatible serial port. */
#include "types.h"
#include "memlayout.h"
#include "defs.h"

#define UART_THR 0
#define UART_LSR 5
#define UART_LSR_EMPTY (1 << 5)

static volatile uchar *const uart = (uchar *)UART0;

void console_init(void) {
  /* OpenSBI/QEMU already configures the UART enough for polling output. */
}

void console_putchar(int ch) {
  if (ch == '\n') {
    console_putchar('\r');
  }
  while ((uart[UART_LSR] & UART_LSR_EMPTY) == 0) {
  }
  uart[UART_THR] = (uchar)ch;
}

void console_puts(const char *s) {
  while (*s) {
    console_putchar(*s++);
  }
}
