/* Tunable constants for the teaching kernel. */
#ifndef TOYOS_PARAM_H
#define TOYOS_PARAM_H

#define NPROC        16
#define NOFILE       8
#define PGSIZE       4096ULL
#define MAXPATH      64
#define PROC_NAME    16
#define USER_TOP     (1ULL << 20)
#define USER_STACK_PAGES 1

#endif
