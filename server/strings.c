#include "strings.h"
#include <stdarg.h>
#include <stdio.h>
#include "memory.h"

// the function will overwrite the buffer pointer, and will 
// allocate memory on its own, no pre-allocation required, 
// but caller must free.
// returns the size of the string written
long strings_svprintf(char **buffer, char *string, va_list list)
{
    long size   = 0;
    char *pointer   = NULL;

    va_list copy;
    va_copy(copy, list);
    size = vsnprintf(pointer, size, string, list);

    if (size < 0)
        return -1;

    size++;             /* For '\0' */
    pointer = m_calloc(size, MEMORY_FILE_LINE);
    if (pointer == NULL)
        return -1;

    size = vsnprintf(pointer, size, string, copy);
    
    va_end(list);
    va_end(copy);

    if (size < 0) {
        m_free(pointer, MEMORY_FILE_LINE);
        return -1;
    }

    *buffer = pointer;
    return(size);
}

long strings_sprintf(char **buffer, char *string, ...)
{
    va_list args;
    va_start(args, string);
    long size   = strings_svprintf(buffer, string, args);
    va_end(args);
    return(size);
}

int strings_count_until(char *buffer, long length, char c)
{
    int count   = 0;
    while(*(buffer+count) != c && count < length)
        count++;
    return(count);
}

void strings_to_lowercase(void *memory, int length)
{
    for(int i=0; i<length; ++i) {
        char c  = *(char*)(memory + i);

        *(char*)(memory + i) = c;
        if(c >= 'A' && c <='Z') {
            *(char*)(memory + i) |= 32;
        }
    }
}