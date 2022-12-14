#include <mm/vmm.h>


volatile struct limine_hhdm_request g_hhdm_request = {
  .id = LIMINE_HHDM_REQUEST,
  .revision = 0
};

#if defined(__x86_64__)

#endif // __x86_64__
