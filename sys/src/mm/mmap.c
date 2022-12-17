/*
 *  Description: Kernel memory mapping functions.
 *  Author(s): Ian Marco Moffett
 *
 */

#include <mm/mmap.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <lib/math.h>
#include <lib/assert.h>

#if defined(__x86_64__)
static uint64_t 
prot_to_pte_flags(int prot) 
{
  uint64_t pte_flags = PTE_NX;

  if (prot & PROT_EXEC) 
  {
    pte_flags &= ~(PTE_NX);
  }

  if (prot & PROT_READ) 
  {
    pte_flags |= PTE_PRESENT;
  }

  if (prot & PROT_WRITE) 
  {
    pte_flags |= PTE_WRITABLE;
  }

  if (prot & PROT_USER) 
  {
    pte_flags |= PTE_USER;
  }

  return pte_flags;
}


static void 
map_pages(vaddr_t vaddr, size_t n_pages, int pte_flags)
{
  pagemap_t top_level = vmm_get_cr3_val();
  for (size_t i = 0; i < n_pages; ++i) 
  {
    paddr_t paddr = pmm_alloc(1);
    ASSERT(paddr != 0, "PADDR IS NULL!\n");

    vmm_map_page(top_level, ALIGN_UP(vaddr, PAGE_SIZE), paddr, pte_flags);
    vaddr += PAGE_SIZE;
  }
}


void* 
k_mmap(void* addr, size_t n_pages, int prot, int flags)
{
  (void)flags;                  /* TODO: Implement this */
  
  if (addr == NULL) 
  {
    /*
     * If the address is not specified, n_pages 
     * pages
     */
    addr = vmm_alloc(n_pages);
    ASSERT(addr != NULL, "ADDR IS NULL\n");
  }

  map_pages((vaddr_t)addr, n_pages, prot_to_pte_flags(prot));
  return addr;
}

#endif
