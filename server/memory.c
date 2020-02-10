#include "memory.h"
#include "defines.h"

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#if DEBUG_MEMORY == 1
static malloc_store_s store = {0};

void add_to_store(void * address
                    , int size_requested
                    , char *function_name
                    , unsigned int fn_name_length)
{
    if(store.allocations.raw_size == 0)
        store.allocations = 
                list_new(MEMORY_ALLOCATION_TABLE_SIZE , sizeof(mdbg_alloc_s));

    if(store.allocation_hashes.size == 0)
        store.allocation_hashes = hash_table_init();

    if(fn_name_length > MEMORY_ALLOCATION_FNNAME_LENGTH)
        fn_name_length = MEMORY_ALLOCATION_FNNAME_LENGTH;


    mdbg_alloc_s allocation = {0};
    allocation.request_type = MEMORY_ALLOCATION_MALLOC;

    memcpy(allocation.function_name, function_name
            , fn_name_length);
        
    allocation.size_requested   = size_requested;
    allocation.address          = address;

    unsigned int index = list_push(&store.allocations
                                    , (void*) &allocation);
    hash_table_bucket_s hash_entry = {(unsigned long)address,  index, 0};
    hash_table_add(&store.allocation_hashes, hash_entry);
}

void update_in_store(void * address
                        , void *original_address
                        , int size_requested
                        , char *function_name
                        , unsigned int fn_name_length
                        , memalloc_enum request_type)
{
    hash_table_bucket_s entry    = 
        hash_table_get(store.allocation_hashes
                        , (unsigned long) original_address);
    
    if(entry.is_occupied != TRUE) {
        printf("Could not update store for address %p, failed!\n", address);
        return;
    }

    int index   = entry.value;
    mdbg_alloc_s *alloc = (mdbg_alloc_s*)list_get(store.allocations, index);
    
    alloc->request_type     = request_type;
    alloc->original_address = original_address;
    alloc->address          = address;

    if(size_requested != -1)
        alloc->size_requested   = size_requested;

    memset(alloc->function_name, 0, sizeof(alloc->function_name));
    memcpy(alloc->function_name, function_name, fn_name_length);

    if(address != NULL) {
        hash_table_bucket_s hash_entry = {(unsigned long)address,  index, 0};
        hash_table_add(&store.allocation_hashes, hash_entry);
    }
}

void *m_malloc(size_t size, char *fn_name)
{
    void *memory    = malloc(size);
    if(memory == NULL)
        exit(MALLOC_FAILED);

    add_to_store(memory, size, fn_name, strlen(fn_name));

    return(memory);
}

void *m_calloc(size_t size, char *fn_name)
{
    void *memory    = calloc(size, 1);
    if(memory == NULL)
        exit(MALLOC_FAILED);

    add_to_store(memory, size, fn_name, strlen(fn_name));

    return(memory);
}

void *m_realloc(void *address, int size, char *fn_name)
{
    void *memory    = realloc(address, size);

    update_in_store(memory, address, size, fn_name, strlen(fn_name)
                            , MEMORY_ALLOCATION_REALLOC);

    return(memory);
}

void m_free(void *address, char *fn_name)
{
    free(address);

    update_in_store(0, address, -1, fn_name
                    , strlen(fn_name), MEMORY_ALLOCATION_FREE);
}

void m_print_dbg()
{
    printf("MEMDBG ALLOCATIONS:\n\n");
    for(size_t i=0; i<store.allocations.index; ++i){
        mdbg_alloc_s j = (*(mdbg_alloc_s*)list_get(store.allocations, i));
        printf("MEMDBG allocation # %ld ::\naddress: %p\n"
                    "allocating function: %s\nrequest type: %d\n"
                    "requested size: %d\noriginal address (if "
                    "applicable): %p\n", i, j.address, j.function_name
                    , j.request_type, j.size_requested, j.original_address);

        printf("*** memory at the location %p starts with ***\n%.*s\n*** ***\n\n"
                    , j.address
                    , j.size_requested > 100 ? 100: j.size_requested
                    , (char*) j.address);
    }
}

#else

void inline m_free(void *address, char *fn_name)
{
    free(address);
}

void inline *m_malloc(size_t size, char *fn_name)
{
    return malloc(size);
}

void inline *m_realloc(void *address, int size, char *fn_name)
{
    return realloc(address, size);
}
void inline *m_calloc(size_t size, char *fn_name)
{
    return calloc(size, 1);
}

#endif //DEBUG_MEMORY == 1