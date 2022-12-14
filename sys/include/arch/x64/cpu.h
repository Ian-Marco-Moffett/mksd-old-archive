#ifndef CPU_H_
#define CPU_H_

#include <lib/types.h>

#if defined(__x86_64__)
uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t val);
#endif  // __x86_64__


#endif
