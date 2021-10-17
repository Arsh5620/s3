#ifndef BSEARCH_INCLUDE_GAURD
#define BSEARCH_INCLUDE_GAURD

#include <stdlib.h>

#define BINARYSEARCH_NORESULT -1

size_t
binary_search (
  char *address, size_t block, size_t count, char *element, size_t length, size_t (*compare) (char *, char *, size_t));
#endif