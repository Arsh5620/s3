#include "strings.h"

int strings_count_until(char *buffer, long length, char c)
{
    int count   = 0;
    while(*(buffer+count) != c && count < length)
        count++;
    return(count);
}

void tolowercase(void *memory, int length)
{
    for(int i=0; i<length; ++i) {
        char c  = *(char*)(memory + i);
        *(char*)(memory + i) =  c >= 'A' && c <='Z' ? c | 32 : c;
    }
}