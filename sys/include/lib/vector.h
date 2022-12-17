#ifndef VECTOR_H_
#define VECTOR_H_


#include <lib/types.h>
#include <mm/heap.h>

#define VECTOR_INIT {0}
#define VECTOR_TYPE(type)                       \
  struct {                                      \
    size_t len;                                 \
    type* elements;                             \
  }


#define VECTOR_TOP(vec_val) vec_val.elements[vec_val.len - 1]
#define VECTOR_IS_EMPTY(vec_val) vec_val.len == 0
#define VECTOR_ELEMENT_COUNT(vec_val) vec_val.len

#define VECTOR_PUSH(vector_ptr, element) do {                                        \
    __auto_type vec = vector_ptr;                                                    \
    if (vec->elements == NULL) {                                                     \
      vec->elements = kmalloc(sizeof(*vec->elements));                                \
      vec->len = 0;                                                                  \
    }                                                                                \
                                                                                     \
    vec->elements[vec->len++] = element;                                             \
    vec->elements = krealloc(vec->elements, sizeof(*vec->elements) * (vec->len + 2)); \
  } while (0);


#define VECTOR_INSERT_AT(vector_ptr, element, idx) do {                              \
    __auto_type vec = vector_ptr;                                                    \
                                                                                     \
    if (vec->elements == NULL) {                                                     \
      vec->elements = kmalloc(sizeof(*vec->elements) * (idx + 1));                    \
    } else {                                                                         \
      vec->elements = krealloc(vec->elements, sizeof(*vec->elements) * (idx + 1));    \
    }                                                                                \
                                                                                     \
    vec->len = idx;                                                                  \
    vec->elements[vec->len++] = element;                                             \
    vec->elements = krealloc(vec->elements, sizeof(*vec->elements) * (idx + 2));      \
 } while (0);


#define VECTOR_POP(vector_ptr, out_ptr) do {                                           \
    __auto_type vec = vector_ptr;                                                      \
    if (vec->len > 0) {                                                                \
      *out_ptr = vec->elements[--vec->len];                                            \
      vec->elements = krealloc(vec->elements, sizeof(*vec->elements) * (vec->len + 1)); \
    }                                                                                  \
 } while (0);

#define VECTOR_POP_AT(vector_ptr, out_ptr, idx) do {                                      \
    __auto_type vec = vector_ptr;                                                         \
    *out_ptr = vec->elements[idx];                                                        \
    for (size_t i = idx; i < vec->len; i += 2) {                                          \
      vec->elements[i] = vec->elements[i + 1];                                            \
    }                                                                                     \
    --vec->len;                                                                           \
    vec->elements = krealloc(vec->elements, sizeof(*vec->elements) * vec->len);            \
  } while (0);

#define VECTOR_READ_AT(vector_ptr, out_ptr, idx) do {       \
    __auto_type vec = vector_ptr;                           \
    if (vec->elements != NULL) {                            \
      *out_ptr = vec->elements[idx];                        \
    }                                                       \
  } while (0);

#define VECTOR_DESTROY(vector_ptr) do {            \
   __auto_type vec = vector_ptr;                   \
   kfree(vec->elements);                           \
   vec->elements = NULL;                           \
   vec->len = 0;                                   \
 } while (0);

#endif // VECTOR_H_

