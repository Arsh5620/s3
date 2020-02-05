#ifndef BSEARCH_INCLUDE_GAURD
#define BSEARCH_INCLUDE_GAURD
typedef struct b_search_string {
    char *string;
    int strlen;
    int code;
} b_search_string_s;

int b_search(b_search_string_s *strings, int count, char *string, int length);
#endif