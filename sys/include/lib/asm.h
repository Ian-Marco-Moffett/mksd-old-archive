/*
 *  Description: ASM related helper functions.
 *  Author(s): Ian Marco Moffett
 *
 */

#ifndef ASM_H_
#define ASM_H_


#define _noreturn __attribute__((noreturn))
#define _packed __attribute__((packed))
#define _unused  __attribute__((unused))
#define _isr __attribute__((interrupt))
#define asmv(code) __asm__ __volatile__(code)
#define asmvf(code, ...) __asm__ __volatile(code, __VA_ARGS__)

#define CLI_SLEEP                               \
  for (uint64_t i = 0; i < 100000000; ++i) {    \
    asmv("cli");                                \
  }


#endif
