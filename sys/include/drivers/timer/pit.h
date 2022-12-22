#ifndef PIT_H_
#define PIT_H_

#include <lib/types.h>


#define DEFAULT_PIT_FREQ 500

void init_pit(void);
void set_pit_count(uint16_t count);

#endif
