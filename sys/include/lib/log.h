#ifndef LOG_H_
#define LOG_H_

#include <stdarg.h>
#include <lib/types.h>

#define PRINTK_INFO "\033[1;37m[ \033[1;36mINFO \033[1;37m] "
#define PRINTK_WARN "\033[1;37m[ \033[1;35mWARN \033[1;37m] "
#define PRINTK_ERROR "\033[1;37m[ \033[1;31mERR \033[1;37m ] "
#define PRINTK_KPANIC_START "\033[1;37m*** \033[1;31m KERNEL PANIC \033[1;37m***\n"
#define PRINTK_KPANIC "\033[1;31mkpanic:\033[1;37m "

void printk(const char* fmt, ...);

#endif
