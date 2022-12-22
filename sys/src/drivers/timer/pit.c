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
  set_pit_count(DEFAULT_PIT_COUNT);
  register_irq(0, pit_isr);
}


void
set_pit_count(uint16_t count)
{
  outb(0x43, 0x34);
  outb(0x40, (uint8_t)count);
  outb(0x40, (uint8_t)(count >> 8));
}

void
pit_sleep(uint32_t n_ticks)
{
  uint32_t e_ticks = ticks + n_ticks;
  while (ticks < e_ticks);
}


size_t
pit_get_count(void)
{
  outb(0x43, 0x34);
  uint8_t lo = inb(0x40);
  uint8_t hi = inb(0x40) << 8;
  return ((uint16_t)hi << 8) | lo;
}
