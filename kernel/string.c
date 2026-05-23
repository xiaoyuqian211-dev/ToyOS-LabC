/* Freestanding memory and string helpers used by the kernel. */
#include "types.h"

void *memset(void *dst, int c, uint64 n) {
  uchar *p = (uchar *)dst;
  while (n-- > 0) {
    *p++ = (uchar)c;
  }
  return dst;
}

void *memmove(void *dst, const void *src, uint64 n) {
  uchar *d = (uchar *)dst;
  const uchar *s = (const uchar *)src;
  if (s < d && d < s + n) {
    d += n;
    s += n;
    while (n-- > 0) {
      *--d = *--s;
    }
  } else {
    while (n-- > 0) {
      *d++ = *s++;
    }
  }
  return dst;
}

void *memcpy(void *dst, const void *src, uint64 n) {
  return memmove(dst, src, n);
}

int memcmp(const void *a, const void *b, uint64 n) {
  const uchar *pa = (const uchar *)a;
  const uchar *pb = (const uchar *)b;
  for (uint64 i = 0; i < n; i++) {
    if (pa[i] != pb[i]) {
      return pa[i] - pb[i];
    }
  }
  return 0;
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
  return (uchar)*a - (uchar)*b;
}

int strncmp(const char *a, const char *b, uint64 n) {
  for (uint64 i = 0; i < n; i++) {
    if (a[i] != b[i] || a[i] == 0 || b[i] == 0) {
      return (uchar)a[i] - (uchar)b[i];
    }
  }
  return 0;
}

char *strncpy(char *dst, const char *src, uint64 n) {
  uint64 i = 0;
  for (; i < n && src[i]; i++) {
    dst[i] = src[i];
  }
  for (; i < n; i++) {
    dst[i] = 0;
  }
  return dst;
}
