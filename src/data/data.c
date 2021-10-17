#include "data.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

size_t
data_string_compare (char *memory, char *source, size_t len)
{
    char *string = memory;
    int string_len = strlen (string);

    int cmp = memcmp (source, string, len); // order does matter it is a - b

    if (cmp == 0 && string_len > len) {
        cmp--;
    } else if (cmp == 0 && string_len < len) {
        cmp++;
    }
    return (cmp);
}