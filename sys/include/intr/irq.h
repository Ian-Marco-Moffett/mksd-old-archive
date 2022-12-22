#ifndef IRQ_H_
#define IRQ_H_

#include <lib/types.h>

void mask_irq(uint8_t irq_line);
void unmask_irq(uint8_t irq_line);

#endif
