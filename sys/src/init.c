/*
 *  Description: Mkall kernel entrypoint.
 *  Author(s): Ian Marco Moffett
 *
 *
 */


#include <lib/limine.h>
#include <lib/asm.h>

_noreturn int main(void) {
  asmv("cli; hlt");
  __builtin_unreachable();
}
