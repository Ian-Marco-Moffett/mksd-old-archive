/*
 *  Description: Interrupt descriptor table.
 *  Author(s): Ian Marco Moffett
 *
 */


#include <arch/x64/idt.h>

#define TRAP_GATE_FLAGS 0x8F
#define INT_GATE_FLAGS 0x8E
#define IDT_INT_GATE_USER 0xEE

#if defined(__x86_64__)
static idt_gate_desc_t idt[256];
static idtr_t idtr = {
  .limit = sizeof(idt_gate_desc_t) * 256 - 1,
  .base = (uint64_t)&idt
};


static void 
set_desc(uint8_t vector, void(*isr)(void* stackframe), uint8_t flags) 
{
  uintptr_t addr = (uintptr_t)isr;
  idt_gate_desc_t* vec = &idt[vector];
  vec->isr_low16 = addr & 0xFFFF;
  vec->isr_mid16 = (addr >> 16) & 0xFFFF;
  vec->isr_high32 = (addr >> 32);
  vec->cs = 0x28;
  vec->ist = 0;
  vec->attr = flags;
  vec->zero = 0;
  vec->zero1 = 0;
  vec->dpl = 3;
  vec->p = 1;
}

void 
register_exception_handler(uint8_t vector, void(*isr)(void* stackframe)) 
{
  set_desc(vector, isr, TRAP_GATE_FLAGS);
}

void 
register_interrupt(uint8_t vector, void(*isr)(void* stackframe))
{
  set_desc(vector, isr, INT_GATE_FLAGS);
}

void 
load_idt(void) 
{
  asmv("lidt %0" :: "m" (idtr));
}

#endif  // __x86_64__
