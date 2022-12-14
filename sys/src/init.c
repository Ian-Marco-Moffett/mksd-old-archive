/*
 *  Description: Mkall kernel entrypoint.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <lib/limine.h>
#include <lib/asm.h>

_noreturn int _start(void) {
  asmv("cli; hlt");
  __builtin_unreachable();
}
