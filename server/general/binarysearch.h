#ifndef BSEARCH_INCLUDE_GAURD
#define BSEARCH_INCLUDE_GAURD

#include <stdlib.h>

#define BINARYSEARCH_NORESULT   -1

size_t binary_search(void *memory, size_t block_length
	, size_t count, char *str, size_t strlen
	, size_t (*compare)(void *, char *, size_t));
#endif