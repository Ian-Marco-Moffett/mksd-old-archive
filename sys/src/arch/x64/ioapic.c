#include <arch/x64/ioapic.h>
#include <acpi/acpi.h>


// Memory mapped register for I/O APIC access.
#define IOREGSEL                        0x00
#define IOWIN                           0x10

// Other I/O APIC registers.
#define IOAPICID                        0x00
#define IOAPICVER                       0x01
#define IOAPICARB                       0x02
#define IOREDTBL                        0x10


static uint32_t
read_mmio(uint8_t reg)
{
  *(volatile uint32_t*)g_ioapic_mmio_base = reg;
  return *(volatile uint32_t*)((uint64_t)g_ioapic_mmio_base + IOWIN);
}

static void
write_mmio(uint8_t reg, uint32_t value)
{
  *(volatile uint32_t*)g_ioapic_mmio_base = reg;
  *(volatile uint32_t*)((uint64_t)g_ioapic_mmio_base + IOWIN) = value;
}

void 
ioapic_set_entry(uint8_t index, uint64_t data) 
{
  write_mmio(IOREDTBL + index * 2, (uint32_t)data);
  write_mmio(IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32));
}

void 
ioapic_init(void)
{
  uint32_t max_redirection_entries = ((read_mmio(IOAPICVER) >> 16) & 0xFF) + 1;

  /* Mask all redirection entries */
  for (uint32_t i = 0; i < max_redirection_entries; ++i)
  {
    ioapic_set_entry(i, 1 << 16);
  }
}
