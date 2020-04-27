#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"
#include "../general/define.h"
#include "../memdbg/memory.h"

char *strings_internal_svprintf(char *string
	, va_list list, long *length, boolean alloc_debug)
{
	long size   = 0;
	char *pointer   = NULL;

	va_list copy;
	va_copy(copy, list);
	size = vsnprintf(pointer, size, string, list);

	if (size < 0)
	{
		return NULL;
	}

	size++;             /* For '\0' */
	pointer = calloc(1, size);
	if (pointer == NULL)
	{
		return NULL;
	}

	size = vsnprintf(pointer, size, string, copy);
	
	va_end(list);
	va_end(copy);

	if (size < 0) 
	{
		free(pointer);
		return NULL;
	}
	
	if (length != NULL)
	{
		*length = size;
	}
	return (pointer);
}

char *strings_svprintf(char *string, va_list list, long *length)
{
	return (strings_internal_svprintf(string, list, length, TRUE));
}

char *strings_svprintf2(char *string, va_list list, long *length)
{
	return (strings_internal_svprintf(string, list, length, FALSE));
}

char *strings_sprintf(char *string, long *length, ...)
{
	va_list args;
	va_start(args, length);
	char *buffer	= strings_internal_svprintf(string, args, length, TRUE);
	va_end(args);
	return(buffer);
}

char *strings_sprintf2(char *string, long *length, ...)
{
	va_list args;
	va_start(args, length);
	char *buffer	= strings_internal_svprintf(string, args, length, FALSE);
	va_end(args);
	return(buffer);
}

void string_tolower(char *memory, ulong length)
{
	for (int i=0; i<length; ++i) 
	{
		char c  = *(memory + i);
		if (c >= 'A' && c <='Z') 
		{
			*(memory + i) |= STRING_LC_FLAG;
		}
	}
}

string_s strings_new_copy(char *data, ulong length)
{
	string_s string;
	// +1 is for the NULL terminator
	ulong allocation_size	= length + 1;

	string.address	= m_malloc(allocation_size, MEMORY_FILE_LINE);
	memcpy(string.address, data, length);
	*(string.address + length)	= NULL_ZERO;
	string.length		= length;
	string.max_length	= allocation_size;

	return (string);
}

string_s string_new(ulong length)
{
	string_s string = {0};
	string.address	= m_calloc(length, MEMORY_FILE_LINE);
	string.max_length	= length;
	return (string);
}

void string_free(string_s string)
{
	if (string.address)
	{
		m_free(string.address, MEMORY_FILE_LINE);
	}
}
