#include "binarysearch.h"
#include <string.h>

/* 
 * return value: the index of the char* "string" in "strings",
 * and -1 if not found.
 */
size_t binary_search(void *memory, size_t block_length
    , size_t count, char *str, size_t strlen
    , size_t (*compare)(void *, char *, size_t))
{
    size_t min = 0;
    size_t max = count;

    while(min < max){
        size_t split    = (max + min)/2;

        signed int cmp = 
            compare(memory + (split * block_length), str, strlen);

        // if(cmp == 0 && strings[split].strlen > length)
        //     cmp--;
        // else if(cmp == 0 &&  strings[split].strlen < length)
        //     cmp++;

        if(cmp < 0)
            max = split;
        else if (cmp > 0)
            min = split + 1;
        else 
            return(split);
    }
    return(BINARYSEARCH_NORESULT);
}

/*
 * key_code_pair_s compare function to use with binary_search
 */
size_t binary_search_kc_cmp(void *memory, char *str, size_t strlen)
{
    key_code_pair_s *s    = (key_code_pair_s*) memory;
    int cmp = memcmp(str, s->string, strlen); // order does matter
    if(cmp == 0 && s->strlen > strlen)
        cmp--;
    else if(cmp == 0 && s->strlen < strlen)
        cmp++;
    
    return(cmp);
}