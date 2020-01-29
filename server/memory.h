# ifndef MEMORY_INCLUDE_GAURD
# define MEMORY_INCLUDE_GAURD

#include <stddef.h>
#include "list.h"
#include "hash_table.h"

#define DEBUG_MEMORY 1

#if DEBUG_MEMORY == 1
#define MEMORY_ALLOCATION_TABLE_SIZE 2048
#define MEMORY_ALLOCATION_FNNAME_LENGTH 32

typedef enum memory_allocation_type_enum {
    MEMORY_ALLOCATION_MALLOC
    , MEMORY_ALLOCATION_REALLOC
    , MEMORY_ALLOCATION_FREE
} memalloc_enum;

typedef struct memory_allocation_table {
    char function_name[MEMORY_ALLOCATION_FNNAME_LENGTH];
    void *address;
    void *original_address;
    int size_requested; 
    memalloc_enum request_type;
} mdbg_alloc_s;

typedef struct malloc_all_store {
    array_list_s allocations;
    hash_table_s allocation_hashes;
} malloc_store_s;

#endif

void m_print_dbg();

void m_free(void *address, char *fn_name);
void *m_malloc(size_t size, char *fn_name);
void *m_calloc(size_t size, char *fn_name);
void *m_realloc(void *address, int size, char *fn_name);

# endif