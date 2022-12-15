#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H 1

#ifdef _KERNEL
#include <lib/types.h>
#else
#include <stdint.h>
#endif


#ifndef __off_t_defined
#ifndef __USE_FILE_OFFSET64
typedef size_t off_t;
#else
typedef size_t off_t;
#endif   // __USE_FILE_OFFSET64
#define __off_t_defined
#endif   // __off_t_defined

#define MAP_FAILED ((void*)-1)
#define PROT_EXEC  (1 << 0)
#define PROT_READ  (1 << 1)
#define PROT_WRITE (1 << 2)

#ifdef _KERNEL
#define PROT_USER  (1 << 3)
#endif


#endif
