# ifndef STRINGS_INCLUDE_GAURD
# define STRINGS_INCLUDE_GAURD

#include <stdarg.h>
#include <stdlib.h>

typedef struct string_struct {
	void *address;
	size_t length;
} string_s;

#define STRINGS_BUFFER_SVNPRINTF 256
#define STRINGS_EMPTY(x) (x.address == NULL || x.length < 1)

/*
 * calloc is _length + 1 for NULL byte
 */
#define STRING_S_MALLOC(_string, _data, _length) \
{ \
	_string.address=m_calloc(_length + 1, MEMORY_FILE_LINE);\
	memcpy(_string.address, _data, _length);\
	_string.length=_length;\
}

long strings_svprintf(char **buffer, char *string, va_list list);
long strings_sprintf(char **buffer, char *string, ...);
void strings_to_lowercase(void *memory, int length);
int strings_count_until(char *buffer, long length, char c);

#endif