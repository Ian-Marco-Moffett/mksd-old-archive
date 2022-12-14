/*
 *  Description: String library.
 *  Author(s): Ian Marco Moffett
 *             Quinn Stephens
 *
 */


#include <lib/string.h>
#include <lib/types.h>
#include <lib/asm.h>

#if defined(__x86_64__)
size_t strlen(const char* str) {
    const char* s2 = str;
    while(*s2) s2++;

    return (s2 - str);
}

char* hex2str(uint64_t hex_num) {
    static char hex_string[80];
    char* ascii_numbers = "0123456789ABCDEF";
    uint8_t nibble;
    uint8_t i = 0, j, temp;
    uint8_t pad = 0;

    // If passed in 0, print a 0
    if (hex_num == 0) {
        memcpy(hex_string, "0\0", 2);
        i = 1;
    }

    if (hex_num < 0x10) pad = 1;    // If one digit, will pad out to 2 later

    while (hex_num > 0) {
        // Convert hex values to ascii string
        nibble = (uint8_t)hex_num & 0x0F;  // Get lowest 4 bits
        nibble = ascii_numbers[nibble];    // Hex to ascii
        hex_string[i] = nibble;             // Move ascii char into string
        hex_num >>= 4;                     // Shift right by 4 for next nibble
        i++;
    }

    if (pad) hex_string[i++] = '0';  // Pad out string with extra 0

    // Add initial "0x" to front of hex string
    hex_string[i++] = 'x';
    hex_string[i++] = '0';
    hex_string[i] = '\0';   // Null terminate string

    // Number is stored backwards in hex_string, reverse the string by swapping ends
    //   until they meet in the middle
    i--;    // Skip null byte
    for (j = 0; j < i; j++, i--) {
        temp          = hex_string[j];
        hex_string[j] = hex_string[i];
        hex_string[i] = temp;
    }

    // Print hex string
    return hex_string;
}

char* dec2str(size_t number) {
    static char dec_string[80];
    uint8_t i = 0, j, temp;
    uint8_t negative = 0;       // Is number negative?

    if (number == 0) dec_string[i++] = '0'; // If passed in 0, print a 0
    else if (number < 0)  {
        negative = 1;       // Number is negative
        number = -number;   // Easier to work with positive values
    }

    while (number > 0) {
        dec_string[i] = (number % 10) + '0';
        number /= 10;
        i++;
    }

    if (negative) dec_string[i++] = '-';

    dec_string[i] = '\0';

    i--;
    for (j = 0; j < i; j++, i--) {
        temp          = dec_string[j];
        dec_string[j] = dec_string[i];
        dec_string[i] = temp;
    }

    return dec_string;
}

void memzero(void* ptr, size_t n) {
  asmvf(
        "cld\n"
        "rep\n"
        "stosb\n" 
        :: "a"(0), "D"(ptr), "c"(n)
    );
}


void memcpy(void* dst, void* src, size_t n) {
  asmvf(
      "cld\n"
      "rep\n"
      "movsb\n" 
      :: "c" (n), "S" (src), "D" (dst)
    );
}


#endif