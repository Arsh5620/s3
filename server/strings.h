# ifndef STRINGS_INCLUDE_GAURD
# define STRINGS_INCLUDE_GAURD

 typedef struct string_struct {
    void *address;
    unsigned long length;
    int error;
} string_s;

void tolowercase(void *memory, int length);
int strings_count_until(char *buffer, long length, char c);

#endif