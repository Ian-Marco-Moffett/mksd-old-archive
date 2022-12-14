/*
 *  Description: Mkall kernel entrypoint.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <lib/limine.h>
#include <lib/asm.h>
#include <lib/log.h>

_noreturn int _start(void) {
  printk(PRINTK_INFO "Booting..\n");
  asmv("cli; hlt");
  __builtin_unreachable();
}
