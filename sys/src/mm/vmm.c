/*
 *  Description: Mkall virtual memory manager.
 *  Author(s): Ian Marco Moffett
 *
 *
 */

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <lib/asm.h>
#include <lib/assert.h>


volatile struct limine_hhdm_request g_hhdm_request = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0
};

#if defined(__x86_64__)
static inline void tlb_flush_single(vaddr_t ptr) {
  asmv("invlpg (%0)" :: "r" (ptr) : "memory");
}


static uintptr_t* get_next_level(pagemap_t* pagemap, uint16_t index, uint8_t do_alloc) {
  if (pagemap[index] & PTE_PRESENT) {
    return ((uintptr_t*)pagemap[index]);
  }

  if (!(do_alloc)) {
    return NULL;
  }

  uintptr_t next_level = pmm_alloc(1);
  ASSERT(next_level != 0, "Failed to allocate frame!\n");

  pagemap[index] = (pagemap_t)next_level | PTE_PRESENT | PTE_WRITABLE;
  return (uintptr_t*)next_level;
}


void vmm_map_page(pagemap_t top_level, vaddr_t vaddr, paddr_t paddr, uint64_t flags) {
  uint64_t pml4_index = (vaddr >> 39) & 0x1FF;
  uint64_t pdpt_index = (vaddr >> 30) & 0x1FF;
  uint64_t pd_index   = (vaddr >> 21) & 0x1FF;
  uint64_t pt_index   = (vaddr >> 12) & 0x1FF;

  uintptr_t* pdpt = get_next_level((pagemap_t*)top_level, pml4_index, 1);
  uintptr_t* pd = get_next_level(pdpt, pdpt_index, 1);
  uintptr_t* pt = get_next_level(pd, pd_index, 1);
  pt[pt_index] = paddr | flags;
  tlb_flush_single(vaddr);
}


#endif // __x86_64__
