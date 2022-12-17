#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <lib/vector.h>


/* Hashtable iterator */
typedef struct 
{
  const char* key;
  void* value;
} hashmap_iter_t;


typedef struct 
{
  VECTOR_TYPE(hashmap_iter_t*) iters;
  uint8_t used : 1;
} hashmap_entry_t;

typedef struct 
{
  VECTOR_TYPE(hashmap_entry_t*) entries;
} hashmap_t;


uint8_t hashmap_insert(hashmap_t* hashmap, void* obj, const char* key);
void* hashmap_read(hashmap_t* hashmap, const char* key);

#endif
