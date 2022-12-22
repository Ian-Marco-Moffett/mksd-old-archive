#include <drivers/timer/pit.h>
#include <arch/x64/io.h>
#include <arch/x64/idt.h>
#include <arch/x64/lapic.h>
#include <lib/asm.h>


static size_t ticks = 0;

_isr static void
pit_isr(void* stackframe)
{
  ++ticks;
  lapic_send_eoi();
}


void
init_pit(void)
{
  set_pit_count(100);
  register_irq(0, pit_isr);
}


void
set_pit_count(uint16_t count)
{
  int divisor = 1193180/count;
  outb(0x43, 0x36);
  outb(0x40, divisor & 0xFF);
  outb(0x40, divisor >> 8);
}
