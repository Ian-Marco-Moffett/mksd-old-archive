#ifndef HEAP_H_
#define HEAP_H_

#include <lib/types.h>

#define HEAP_SIZE_PAGES 10


void* kmalloc(size_t n_bytes);
void* kcalloc(size_t nmemb, size_t size);
void* krealloc(void* old, size_t new_size);
void kfree(void* ptr);


#endif
