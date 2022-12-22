#include <mm/heap.h>
#include <mm/mmap.h>
#include <mm/vmm.h>
#include <lib/string.h>

#define HEAP_MAG 0xCA7F00D

typedef struct HeapHeader 
{
  uint32_t magic;
  size_t msize_bytes;
  struct HeapHeader* next;
  struct HeapHeader* prev;
} heap_header_t;


static heap_header_t* heap_start = NULL;
static heap_header_t* free_ptr = NULL;
static size_t bytes_allocated = 0;

#if defined(__x86_64__)
static void 
heap_init(void) 
{
  heap_start = k_mmap(NULL, 
                      HEAP_SIZE_PAGES, 
                      PROT_READ 
                      | PROT_WRITE 
                      | PROT_EXEC, 0);

  free_ptr = heap_start;
}


void* 
kmalloc(size_t n_bytes) 
{
  if (heap_start == NULL) 
  {
    heap_init();
  }

  if (bytes_allocated + n_bytes > HEAP_SIZE_PAGES*PAGE_SIZE) 
  {
    return NULL;
  }

  free_ptr->msize_bytes = n_bytes;
  void* next = 
    (void*)((uintptr_t)free_ptr + (sizeof(heap_header_t) + n_bytes));

  free_ptr->magic = HEAP_MAG;
  free_ptr->next = next;
  free_ptr->next->prev = free_ptr;
  free_ptr = next;
  bytes_allocated += n_bytes;
  return (void*)((uintptr_t)next - n_bytes);
}


void*
kcalloc(size_t nmemb, size_t size)
{
  void* block = kmalloc(nmemb * size);
  memzero(block, nmemb * size);
  return block;
}

void 
kfree(void* ptr) 
{
  heap_header_t* hdr = 
    (heap_header_t*)((uintptr_t)ptr - sizeof(heap_header_t));

  if (hdr->prev) 
  {
      hdr->prev->next = hdr->next;
  }

  if (hdr->next) 
  {
    hdr->next->prev = hdr->prev;
  }

  bytes_allocated -= hdr->msize_bytes;
}


void* 
krealloc(void* old, size_t new_size) 
{
  void* new = kmalloc(new_size);
  memcpy(new, old, new_size);
  kfree(old);
  return new;
}

#endif
