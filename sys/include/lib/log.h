#ifndef LOG_H_
#define LOG_H_

#include <stdarg.h>
#include <lib/types.h>

#define PRINTK_RED "\033[0;31m"
#define PRINTK_PANIC PRINTK_RED "kpanic: "

#define PRINTK_INFO "\033[1;37m[ \033[1;36mINFO \033[1;37m] "
#define PRINTK_WARN "\033[1;37m[ \033[1;35mWARN \033[1;37m] "
#define PRINTK_ERROR "\033[1;37m[ \033[1;31mERR \033[1;37m ] "

void printk(const char* fmt, ...);

#endif
