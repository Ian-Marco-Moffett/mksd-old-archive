/*
 *  Description: Mkall kernel entrypoint.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <lib/limine.h>
#include <lib/asm.h>
#include <lib/log.h>
#include <arch/x64/exceptions.h>
#include <arch/x64/idt.h>

_noreturn int _start(void) {
  printk(PRINTK_INFO "Booting..\n");
  load_idt();
  init_exceptions();
  
  asmv("cli; hlt");
  __builtin_unreachable();
}
