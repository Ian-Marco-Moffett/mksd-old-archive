#ifndef VMM_H_
#define VMM_H_

#include <lib/types.h>
#include <lib/limine.h>

#if defined(__x86_64__)
#define VMM_HIGHER_HALF (g_hhdm_request.response->offset)

extern volatile struct limine_hhdm_request g_hhdm_request;


typedef uintptr_t pagemap_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000
#define PTE_PRESENT (1ull << 0)
#define PTE_WRITABLE (1ull << 1)
#define PTE_USER (1ULL << 2)
#define PTE_NX (1ULL << 63)
#define PTE_GET_ADDR(VALUE) 

#endif // __x86_64__
#endif // VMM_H_
