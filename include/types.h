/* Basic fixed-width types used by the ToyOS kernel and user ABI. */
#ifndef TOYOS_TYPES_H
#define TOYOS_TYPES_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned int uint32;
typedef int int32;

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif
