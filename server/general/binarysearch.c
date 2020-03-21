#include "binarysearch.h"
#include <string.h>

/* 
 * return value: the index of the char* "string" in "strings",
 * and -1 if not found.
 */
int b_search(b_search_string_s *strings, int count, char *string, int length)
{
    int min = 0;
    int max = count;

    while(min < max){
        int split    = (max + min)/2;

        int cmp = memcmp(string, strings[split].string, length);

        if(cmp == 0 && strings[split].strlen > length)
            cmp--;
        else if(cmp == 0 &&  strings[split].strlen < length)
            cmp++;

        if(cmp < 0)
            max = split;
        else if (cmp > 0)
            min = split + 1;
        else 
            return(split);
    }
    return(-1);
}