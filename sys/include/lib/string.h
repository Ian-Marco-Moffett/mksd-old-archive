#ifndef STRING_H_
#define STRING_H_

#include <lib/types.h>
#include <stdarg.h>


#if defined(__x86_64__)
void memzero(void* ptr, size_t n);
void memset(void* ptr, uint8_t byte, size_t n);
void memcpy(void* dst, const void* src, size_t n);
char* dec2str(size_t number);
char* hex2str(uint64_t hex_num);
void strappend(char* str1, const char* str2);
char* strsplit(const char* str, char dilm, size_t idx);
size_t strlen(const char* str);
size_t strdilm_count(const char* str, char dilm);
uint8_t strcmp(const char* str1, const char* str2);
uint8_t memcmp(const char* str1, const char* str2, size_t n);
void snprintf(char* str, size_t size, const char* fmt, ...);
#endif  // __x86_64__


#endif // STRING_H_
