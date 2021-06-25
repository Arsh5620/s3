#include "binarysearch.h"
#include <stdio.h>
#include <string.h>

/*
 * returns the index of the element in the given array by
 * performing binary search on the address with "compare"
 */
size_t
binary_search (
  char *address, size_t block, size_t count, char *element, size_t length, size_t (*compare) (char *, char *, size_t))
{
    size_t min = 0;
    size_t max = count;

    while (min < max) {
        size_t split = (max + min) / 2;

        char *index = (address + (split * block));
        signed int result = compare (*(char **) index, element, length);

        if (result < 0) {
            max = split;
        } else if (result > 0) {
            min = split + 1;
        } else {
            return (split);
        }
    }

    printf ("no match found\n");
    return BINARYSEARCH_NORESULT;
}