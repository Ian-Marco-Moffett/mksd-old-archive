#ifndef STRING_H_
#define STRING_H_

#include <lib/types.h>


#if defined(__x86_64__)
void memzero(void* ptr, size_t n);
void memset(void* ptr, uint8_t byte, size_t n);
void memcpy(void* dst, void* src, size_t n);
char* dec2str(size_t number);
char* hex2str(uint64_t hex_num);
size_t strlen(const char* str);
uint8_t strcmp(const char* str1, const char* str2);
uint8_t memcmp(const char* str1, const char* str2, size_t n);
#endif  // __x86_64__


#endif // STRING_H_
