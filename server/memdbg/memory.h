# ifndef MEMORY_INCLUDE_GAURD
# define MEMORY_INCLUDE_GAURD

#include <stddef.h>
#include "../data-structures/list.h"
#include "../data-structures/hash_table.h"
#include "../data-structures/linked_list.h"

#define MEMORY_FILE_LINE __BASE_FILE__, __LINE__

#ifdef DEBUG

#define MEMORY_TABLE_SIZE   2048

typedef enum {
	MEMORY_ALLOC_MALLOC
	, MEMORY_ALLOC_CALLOC
	, MEMORY_ALLOC_REALLOC
	, MEMORY_ALLOC_FREE
} malloc_enum;

typedef struct memory_allocation_update {
	char *file_name; // please use the compiler directive __FILE__
	long size; 
	long line_no; // please use compiler directive __LINE__
	malloc_enum type;
} malloc_update_s;

typedef struct memory_allocation_table {
	char *address;
	char *new_addr;	/* only used with reallocations */
	malloc_enum	last_update;
	linked_list_s updates;
} malloc_node_s;

typedef struct {
	my_list_s list;
	hash_table_s hash;
	char is_init;
} malloc_s;


void memory_cleanup();
char *memory_log_gettype(int i);
void memory_log_handle(malloc_enum type
	, malloc_node_s *node, malloc_update_s *update);
long memory_get_allocation_size(malloc_node_s *node);
#endif

void m_free(void *address, char *file_name, long line_no);
void *m_malloc(size_t size, char *file_name, long line_no);
void *m_calloc(size_t size, char *file_name, long line_no);
void *m_realloc(void *address, long size, char *file_name, long line_no);

#endif