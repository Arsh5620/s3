typedef struct b_search_string {
    char *string;
    int strlen;
} b_search_string_s;

int b_search(b_search_string_s *strings, int count, char *string, int length);