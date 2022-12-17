/*
 *  Description: This module allows interaction with the CPU.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <arch/x64/cpu.h>
#include <lib/asm.h>

#if defined(__x86_64__)

uint64_t 
rdmsr(uint32_t msr) 
{
  uint32_t lo, hi;
  asmvf("rdmsr" : "=a" (lo), "=d" (hi) : "c" (msr));
  return ((uint64_t)hi << 32 | lo);
}


void 
wrmsr(uint32_t msr, uint64_t val) 
{
  uint32_t lo = val & 0xFFFFFFFF;
  uint32_t hi = val >> 32;
  asmvf("wrmsr" :: "a" (lo), "d" (hi), "c" (msr));
}

#endif  // __x86_64__
