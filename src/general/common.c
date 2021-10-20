#include "common.h"

void
string_hexify (uchar *dest, uchar *source, long strlen)
{
    for (long i = 0; i < strlen; ++i) {
        *(dest + i * 2) = (*(source + i) >> 4) & 0xF;
        *(dest + i * 2 + 1) = *(source + i) & 0xF;
    }

    for (long i = 0; i < strlen * 2; ++i) {
        *(dest + i) = uchar_to_hex (*(dest + i));
    }
}

uchar inline uchar_to_hex (uchar digit)
{
    if (digit < 10) {
        return (digit + '0');
    } else if (digit < 16) {
        return digit - 10 + 'a';
    }
    
    return '0';
}