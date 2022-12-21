#ifndef ACPI_H_
#define ACPI_H_

#include <lib/types.h>

extern void* g_lapic_mmio_base;
extern void* g_ioapic_mmio_base;

void acpi_init(void);
void acpi_power_off(void);


#endif
