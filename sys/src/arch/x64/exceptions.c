/*
 *  Description: An exception handling module.
 *  Author(s): Ian Marco Moffett
 *
 */

#include <arch/x64/exceptions.h>
#include <arch/x64/idt.h>
#include <lib/log.h>

_isr static void vec_0x0(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Division by zero\n");
  asmv("cli; hlt");
}


_isr static void vec_0x1(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Debug exception\n");
  asmv("cli; hlt");
}


_isr static void vec_0x3(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Breakpoint exception\n");
  asmv("cli; hlt");
}


_isr static void vec_0x4(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Overflow exception\n");
  asmv("cli; hlt");
}


_isr static void vec_0x5(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "BOUND range exceeded\n");
  asmv("cli; hlt");
}

_isr static void vec_0x6(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Invalid opcode\n");
  asmv("cli; hlt");
}


_isr static void vec_0x8(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Double fault\n");
  asmv("cli; hlt");
}


_isr static void vec_0xA(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Invalid TSS\n");
  asmv("cli; hlt");
}


_isr static void vec_0xB(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Segment not present\n");
  asmv("cli; hlt");
}

_isr static void vec_0xC(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Stack segment fault\n");
  asmv("cli; hlt");
}


_isr static void vec_0xD(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "General protection fault\n");
  asmv("cli; hlt");
}


_isr static void vec_0xE(void* stackframe) {
  asmv("cli");
  printk(PRINTK_KPANIC_START);
  printk(PRINTK_KPANIC "Page fault\n");
  asmv("cli; hlt");
}

void init_exceptions(void) {
  register_exception_handler(0x0, vec_0x0);
  register_exception_handler(0x1, vec_0x1);
  register_exception_handler(0x3, vec_0x3);
  register_exception_handler(0x4, vec_0x4);
  register_exception_handler(0x5, vec_0x5);
  register_exception_handler(0x6, vec_0x6);
  register_exception_handler(0x8, vec_0x8);
  register_exception_handler(0xA, vec_0xA);
  register_exception_handler(0xB, vec_0xB);
  register_exception_handler(0xC, vec_0xC);
  register_exception_handler(0xD, vec_0xD);
  register_exception_handler(0xE, vec_0xE);
}
