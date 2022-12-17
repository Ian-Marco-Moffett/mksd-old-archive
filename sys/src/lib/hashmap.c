#include <lib/hashmap.h>
#include <lib/string.h>
#include <lib/log.h>


static uint32_t hashmap_hash(const void* data, size_t length) {
  const uint8_t* data_u8 = data;
  uint32_t result = 0;

  for (size_t i = 0; i < length; ++i) {
    uint32_t c = data_u8[i];
    result = c + (result << 6) + (result << 16) - result;
  }

  return result;
}


static uint8_t has_duplicates(hashmap_t* hashmap, const char* key) {
  hashmap_entry_t* entry;
  hashmap_iter_t* iter;

  for (size_t i = 0; i < hashmap->entries.len; ++i) {
    VECTOR_READ_AT(&hashmap->entries, &entry, i);
    for (size_t j = 0; j < entry->iters.len; ++j) {
      VECTOR_READ_AT(&entry->iters, &iter, j);
      if (strcmp(iter->key, key) == 0) {
        return 1;
      }
    }
  }

  return 0;
}


static void* find_obj_in_iter(hashmap_entry_t* entry, const char* key) {
  hashmap_iter_t* iter;

  for (size_t i = 0; i < entry->iters.len; ++i) {
    VECTOR_READ_AT(&entry->iters, &iter, i);
    if (strcmp(iter->key, key) == 0) {
      return iter->value;
    }
  }

  return NULL;
}


uint8_t hashmap_insert(hashmap_t* hashmap, void* obj, const char* key) {
  size_t hash = 0;

  if (hashmap->entries.len != 0) {
    hash = hashmap_hash(key, strlen(key)) % hashmap->entries.len;
  } 

  if (has_duplicates(hashmap, key)) {
    return 1;
  }

  /* Find the entry */
  hashmap_entry_t* entry = NULL;
  VECTOR_READ_AT(&hashmap->entries, &entry, hash);

  if (entry == NULL) {
    /* No entries, make one */
    entry = kmalloc(sizeof(hashmap_entry_t));
    VECTOR_PUSH(&hashmap->entries, entry);
  }

  /* Create an insert an iter */
  hashmap_iter_t* iter = kmalloc(sizeof(hashmap_iter_t));
  iter->key = key;
  iter->value = obj;
  VECTOR_PUSH(&entry->iters, iter);
  return 0;
}

void* hashmap_read(hashmap_t* hashmap, const char* key) {
  if (hashmap->entries.len == 0) {
    return NULL;
  }

  size_t hash = hashmap_hash(key, strlen(key)) % hashmap->entries.len;

  /* Find the entry */
  hashmap_entry_t* entry = NULL;
  VECTOR_READ_AT(&hashmap->entries, &entry, hash);

  return find_obj_in_iter(entry, key);
}
