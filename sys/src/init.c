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
#include <drivers/hdd/ahci.h>
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


static void
init_drivers(void)
{
  ahci_init();
}


static void
fs_init(void)
{
  vfs_init();
}


_noreturn void
_start(void) 
{
  printk(PRINTK_INFO "Booting..\n");

#if defined(__x86_64__)
  load_idt();
  init_exceptions();
  init_mm();
#endif
  
  acpi_init();
  fs_init();
  init_drivers();
  asmv("cli; hlt");
  __builtin_unreachable();
}
