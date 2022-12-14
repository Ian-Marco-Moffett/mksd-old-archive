#ifndef IDT_H_
#define IDT_H_

#include <lib/asm.h>
#include <lib/types.h>

#if defined(__x86_64__)
typedef struct {
  uint16_t isr_low16;
  uint16_t cs;
  uint8_t ist   : 2;
  uint8_t zero  : 4;
  uint8_t attr  : 4;
  uint8_t zero1 : 1;
  uint8_t dpl   : 2;
  uint8_t p     : 1;
  uint16_t isr_mid16;
  uint32_t isr_high32;
  uint32_t reserved;
} idt_gate_desc_t;

/* Points to the IDT */
typedef struct {
  uint16_t limit;
  uint64_t base;
} idtr_t;

#endif    // __x86_64__
#endif    // IDT_H_
