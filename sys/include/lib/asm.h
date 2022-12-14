#ifndef ASM_H_
#define ASM_H_


#define _noreturn __attribute__((noreturn))
#define _packed __attribute__((packed))
#define _unused  __attribute__((unused))
#define asmv(code) __asm__ __volatile__(code)


#endif
