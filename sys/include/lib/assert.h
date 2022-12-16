#include <lib/asm.h>
#include <lib/log.h>

#if defined(__x86_64__)
#define ASSERT(condition, failmsg)    \
  if (!(condition)) {                 \
    printk(PRINTK_KPANIC_START);      \
    printk(PRINTK_KPANIC "ASSERTION \"" #condition "\" FAILED: %s (%s:%d)\n", failmsg, __FILE__, __LINE__);     \
    asmv("cli; hlt");                 \
  }
#endif
