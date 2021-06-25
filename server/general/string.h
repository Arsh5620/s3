#ifndef STRING_INCLUDE_GAURD
#define STRING_INCLUDE_GAURD

#include "../general/define.h"
#include <stdarg.h>

typedef struct string {
    char *address;
    long length;
} string_s;

#define STRING_ISEMPTY(string) (string.address == NULL || string.length <= 0)

#define STRING(x)                                                                                                                \
    (string_s) { .address = x, .length = sizeof (x) - 1 }
#define STRING2(x, y)                                                                                                            \
    (string_s) { .address = x, .length = y }

string_s
string_new (ulong length);
string_s
string_new_copy (char *data, ulong length);
void
string_free (string_s string);

char *
string_svprintf (char *string, va_list list, long *length);
// @ same as string_svprintf but no memory tracking
char *
string_svprintf2 (char *string, va_list list, long *length);
char *
string_sprintf (char *string, long *length, ...);
// @ same as string_sprintf but no memory tracking
char *
string_sprintf2 (char *string, long *length, ...);

void
string_tolower (char *memory, ulong length);
#endif