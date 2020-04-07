#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>

#include "./memory.h"
#include "../general/defines.h"
#include "../errors/errorhandler.h"

#ifdef DEBUG_MEMORY

static malloc_s allocations	= {0};

void memory_track_add(void * address
    , long size , char *file_name , long line_no)
{
    if (allocations.is_init == FALSE) 
	{
        allocations.list  = 
            my_list_new(MEMORY_TABLE_SIZE, sizeof(malloc_node_s));
        allocations.hash  = hash_table_init(MEMORY_TABLE_SIZE, 0);
        allocations.is_init   = TRUE;
    }
    
    malloc_node_s track_node = {
    	.address  = address
		, .last_update	= MEMORY_ALLOC_MALLOC
	};

	malloc_update_s node	= {
    	.type = MEMORY_ALLOC_MALLOC
    	, .file_name    = file_name
    	, .line_no  = line_no
    	, .size = size
	};

	linked_list_push(&track_node.updates, (char*)&node, sizeof(node));
    long index = my_list_push(&allocations.list, (char*) &track_node);

    hash_table_bucket_s hash_entry = {
		.key.address	= (void*) address
		, .value.number	=  index
		, .key_len	= 0
		, .value_len	= 0
		, .is_occupied	= 0
	};

    hash_table_add(&allocations.hash, hash_entry);
	memory_log_handle(MEMORY_ALLOC_MALLOC, &track_node, &node);
}

void memory_track_update(void * address, void *new_addr, long size
    , char *file_name , long line_no , malloc_enum type)
{
	hash_input_u key	= { .address = (void*) address };
    hash_table_bucket_s entry   = hash_table_get(allocations.hash, key, 0);
    
    if(entry.is_occupied != TRUE) 
	{
        error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG,
			MEMORY_ALLOCATION_ERROR, MEMORY_ALLOCATION_NOENTRY);
        return;
    }

    malloc_node_s *allocation   = 
        (malloc_node_s*)my_list_get(allocations.list, entry.value.number);

	allocation->last_update	= type;

	malloc_update_s node	= { 
    	.type = type
    	, .file_name    = file_name
    	, .line_no  = line_no
    	, .size = size
	};

	linked_list_push(&allocation->updates, (char*)&node, sizeof(node));

	if (type == MEMORY_ALLOC_FREE) 
	{
		hash_input_u	rmindex	= (hash_input_u){
			.address = address
		};
		hash_table_remove(&allocations.hash, rmindex, 0);
	}
	else if (type == MEMORY_ALLOC_REALLOC 
		&& new_addr != NULL 
		&& address != new_addr) 
	{
		memory_track_add(new_addr, size, file_name, line_no);
		allocation->new_addr	= new_addr;
		malloc_update_s node	= {
    		.type = MEMORY_ALLOC_FREE
    		, .file_name    = file_name
    		, .line_no  = line_no
    		, .size = size
		};
		linked_list_push(&allocation->updates, (char*)&node, sizeof(node));
	}

	memory_log_handle(type, allocation, &node);
}

void *m_malloc(size_t size, char *file_name, long line_no)
{
    void *memory    = malloc(size);
    assert(memory != NULL);
    memory_track_add(memory, size, file_name, line_no);
    return(memory);
}

void *m_calloc(size_t size, char *file_name, long line_no)
{
    void *memory    = calloc(size, 1);
    assert(memory != NULL);
    memory_track_add(memory, size, file_name, line_no);
    return(memory);
}

void *m_realloc(void *address, long size, char *file_name, long line_no)
{
    void *memory    = realloc(address, size);
	size	= memory == NULL ? -1 : size;
    memory_track_update(address, memory
        , size, file_name, line_no, MEMORY_ALLOC_REALLOC);
    return(memory);
}

void m_free(void *address, char *file_name, long line_no)
{
    if (address) 
	{
        free(address);
        memory_track_update(address, address, -1, file_name
            , line_no, MEMORY_ALLOC_FREE);
    }
}

void memory_log_handle(malloc_enum type
	, malloc_node_s *node, malloc_update_s *update)
{
	static char *mem_types[]	= {"malloc", "calloc", "realloc", "free"};
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_DEBUG,
		MEMORY_ALLOCATION_LOG
		, node->address, node->new_addr
		, mem_types[update->type], update->size
		, update->file_name, update->line_no);
}

#else

void inline m_free(void *address, char *file_name, long line_no)
{
    free(address);
}

void inline *m_malloc(size_t size, char *file_name, long line_no)
{
    return malloc(size);
}

void inline *m_realloc(void *address, long size
    , char *file_name, long line_no)
{
    return realloc(address, size);
}
void inline *m_calloc(size_t size, char *file_name, long line_no)
{
    return calloc(size, 1);
}

#endif //DEBUG_MEMORY == 1