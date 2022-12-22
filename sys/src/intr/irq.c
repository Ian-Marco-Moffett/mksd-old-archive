#include <intr/irq.h>
#include <arch/x64/ioapic.h>
#include <acpi/acpi.h>


void 
mask_irq(uint8_t irq_line)
{
  ioapic_set_entry(acpi_remap_irq(irq_line), 1 << 16);
}


void 
unmask_irq(uint8_t irq_line)
{
  ioapic_set_entry(acpi_remap_irq(irq_line), 0x20 + irq_line);
}
