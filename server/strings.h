# ifndef STRINGS_INCLUDE_GAURD
# define STRINGS_INCLUDE_GAURD

#include <stdarg.h>

typedef struct string_struct {
    void *address;
    unsigned long length;
    int error;
} string_s;

#define STRINGS_BUFFER_SVNPRINTF 256

long strings_svprintf(char **buffer, char *string, va_list list);
long strings_sprintf(char **buffer, char *string, ...);
void strings_to_lowercase(void *memory, int length);
int strings_count_until(char *buffer, long length, char c);

#endif