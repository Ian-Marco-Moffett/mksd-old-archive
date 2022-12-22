#ifndef PIT_H_
#define PIT_H_

#include <lib/types.h>


#define DEFAULT_PIT_COUNT 100
#define PIT_DIVIDEND 1193180

void init_pit(void);
void set_pit_count(uint16_t count);
size_t pit_get_count(void);
void pit_sleep(uint32_t n_ticks);

#endif
