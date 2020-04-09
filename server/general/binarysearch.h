#ifndef BSEARCH_INCLUDE_GAURD
#define BSEARCH_INCLUDE_GAURD

#include <stdlib.h>

typedef struct b_search_string {
	char *string;
	int strlen;
	int code;
} key_code_pair_s;

#define BINARYSEARCH_NORESULT   -1

size_t binary_search(void *memory, size_t block_length
	, size_t count, char *str, size_t strlen
	, size_t (*compare)(void *, char *, size_t));
size_t binary_search_kc_cmp(void *memory, char *str, size_t strlen);
#endif