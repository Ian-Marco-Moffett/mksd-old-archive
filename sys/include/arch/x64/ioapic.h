#ifndef IOAPIC_H_
#define IOAPIC_H_

#include <lib/types.h>

void ioapic_init(void);
void ioapic_set_entry(uint8_t index, uint64_t data);

#endif
