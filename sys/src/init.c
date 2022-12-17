/*
 *  Description: MKSD kernel entrypoint.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <lib/limine.h>
#include <lib/asm.h>
#include <lib/log.h>
#include <mm/pmm.h>
#include <acpi/acpi.h>
#include <fs/vfs.h>

#if defined(__x86_64__)
#include <arch/x64/exceptions.h>
#include <arch/x64/idt.h>
#else
#error MKSD only supports x86_64 for now
#endif


static void 
init_mm(void) 
{
  pmm_init();
}


_noreturn int 
_start(void) 
{
  printk(PRINTK_INFO "Booting..\n");

#if defined(__x86_64__)
  load_idt();
  init_exceptions();
  init_mm();
#endif
  
  acpi_init();
  vfs_init();
  asmv("cli; hlt");
  __builtin_unreachable();
}
