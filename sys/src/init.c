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
#include <fs/devfs.h>
#include <fs/ext2.h>
#include <arch/x64/lapic.h>
#include <arch/x64/ioapic.h>

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
pre_fs_init(void)
{
  vfs_init();
  devfs_init();
}


static void
post_fs_init(void)
{
  ext2_init();
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
  pre_fs_init();
  init_drivers();
  post_fs_init(); 

  lapic_init();
  printk(PRINTK_INFO "Local APIC initialized for the BSP.\n");
  ioapic_init();
  printk(PRINTK_INFO "I/O APIC initialized.\n");

  asmv("cli; hlt");
  __builtin_unreachable();
}
