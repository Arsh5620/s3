#ifndef SRC_MEMDBG_MEMORY_
#define SRC_MEMDBG_MEMORY_

#include "data/hash_table.h"
#include "data/linked_list.h"
#include <stddef.h>

void
__free (void *address, char *file_name, long line_no);
void *
__malloc (long size, char *file_name, long line_no);
void *
__calloc (long size, char *file_name, long line_no);
void *
__realloc (void *address, long size, char *file_name, long line_no);

#define m_calloc(size) __calloc (size, __FILE__, __LINE__)
#define m_malloc(size) __malloc (size, __FILE__, __LINE__)
#define m_realloc(pointer, new_size) __realloc (pointer, new_size, __FILE__, __LINE__)
#define m_free(pointer) __free (pointer, __FILE__, __LINE__)

void
memory_debug_print_lost ();

#endif // SRC_MEMDBG_MEMORY_