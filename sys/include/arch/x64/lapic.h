#ifndef LAPIC_H_
#define LAPIC_H_

#include <lib/types.h>

void lapic_init(void);
void lapic_send_eoi(void);
void lapic_timer_calibrate(void);
void lapic_timer_oneshot(uint32_t micro_seconds);
uint32_t lapic_read_id(void);

#endif 
