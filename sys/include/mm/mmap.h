#ifndef MMAP_H_
#define MMAP_H_

#include <sys/mman.h>

#if defined(__x86_64__)
void* k_mmap(void* addr, size_t n_pages, int prot, int flags);
#endif

#endif
