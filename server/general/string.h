# ifndef STRING_INCLUDE_GAURD
# define STRING_INCLUDE_GAURD

#include <stdarg.h>
#include "../general/define.h"

typedef struct string_struct {
	char *address;
	size_t length;
	size_t max_length;
} string_s;

// @ simple if statement to check if string is empty
#define STRING_ISEMPTY(string)	(string.address == NULL || string.length < 1)

#define STRING(x) \
	(string_s){.address = x, .length = sizeof(x), .max_length = sizeof(x)}
// @ lower case flag
#define STRING_LC_FLAG			(0x20)

string_s string_new(ulong length);
string_s string_new_copy(char *data, ulong length);
void string_free(string_s string);

char *string_svprintf(char *string, va_list list, long *length);
// @ same as string_svprintf but no memory tracking
char *string_svprintf2(char *string, va_list list, long *length);
char *string_sprintf(char *string, long *length, ...);
// @ same as string_sprintf but no memory tracking
char *string_sprintf2(char *string, long *length, ...);

void string_tolower(char *memory, ulong length);
#endif