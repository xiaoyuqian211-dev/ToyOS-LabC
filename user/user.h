/* User-space ABI and tiny libc-like helpers for ToyOS programs. */
#ifndef TOYOS_USER_H
#define TOYOS_USER_H

typedef unsigned long long uint64;
typedef unsigned int uint;

long write(int fd, const void *buf, int n);
void exit(int status) __attribute__((noreturn));
int getpid(void);
int yield(void);
int fork(void);
int wait(int *status);
int exec(const char *name);
int readfile(const char *path, char *buf, int max);
int info(void);

int puts(const char *s);
void printf(const char *fmt, ...);
uint64 strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, uint64 n);
char *strcpy(char *dst, const char *src);

#endif
