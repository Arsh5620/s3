# ifndef MEMORY_INCLUDE_GAURD
# define MEMORY_INCLUDE_GAURD

#include <stddef.h>
#include "list.h"
#include "hash_table.h"

#define DEBUG_MEMORY

#define MEMORY_FILE_LINE __FILE__, __LINE__

#ifdef DEBUG_MEMORY

#define MEMORY_TABLE_SIZE   2048

typedef enum {
    MEMORY_ALLOC_MALLOC
    , MEMORY_ALLOC_CALLOC
    , MEMORY_ALLOC_REALLOC
    , MEMORY_ALLOC_FREE
} malloc_enum;

typedef struct memory_allocation_table {
    char *file_name; // please use the compiler directive __FILE__
    void *address;
    void *prev_address;
    long size; 
    long line_no; // please use compiler directive __LINE__
    long counter;
    malloc_enum type;
} malloc_node_s;

typedef struct {
    array_list_s list;
    hash_table_s hash;
    char is_init;
} malloc_s;

#endif

void memory_print_debug();

void m_free(void *address, char *file_name, long line_no);
void *m_malloc(size_t size, char *file_name, long line_no);
void *m_calloc(size_t size, char *file_name, long line_no);
void *m_realloc(void *address, long size, char *file_name, long line_no);

#endif
#define MEMORY_INCLUDE_FINISHED