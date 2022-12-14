/*
 *  Description: Mkall kernel entrypoint.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <lib/limine.h>
#include <lib/asm.h>
#include <lib/log.h>

#if defined(__x86_64__)
#include <arch/x64/exceptions.h>
#include <arch/x64/idt.h>
#else
#error MkallOS only supports x86_64 for now
#endif

_noreturn int _start(void) {
  printk(PRINTK_INFO "Booting..\n");

#if defined(__x86_64__)
  load_idt();
  init_exceptions();
#endif
  
  asmv("cli; hlt");
  __builtin_unreachable();
}
