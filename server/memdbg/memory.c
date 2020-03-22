#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include "./memory.h"
#include "../general/defines.h"

#ifdef DEBUG_MEMORY

// the program is found to have limitations in case the operating system 
// assigned a just-freed block to the new mallocation the address
// for which already exists in the table. 

static malloc_s track = {0}; // only available in the same object file

void memory_exit(int reason)
{
    if(reason   == MALLOC_FAILED)
        printf("malloc");
    if(reason   == CALLOC_FAILED)
        printf("calloc");
    if(reason   == REALLOC_FAILED)
        printf("realloc");

    if(reason == MALLOC_FAILED 
        || reason == CALLOC_FAILED 
        || reason == REALLOC_FAILED)
        printf(" failed possibly due to low memory, program will "
            "now exit, please check logs and program exit code for"
            " more info.\n");
    
    if(reason == FREE_FAILED)
        printf("cannot free memory when tried, program will exit, and "
            "your OS will free all the memory.\n");
    exit(reason);
}

void memory_track_add(void * address
    , long size , char *file_name , long line_no)
{
    if(track.is_init == FALSE){
        track.list  = 
            my_list_new(MEMORY_TABLE_SIZE, sizeof(malloc_node_s));
        track.hash  = hash_table_initn(MEMORY_TABLE_SIZE, 0);
        track.is_init   = TRUE;
    }
    
    malloc_node_s track_node = {0};
    track_node.type = MEMORY_ALLOC_MALLOC;
    
    track_node.file_name    = file_name;
    track_node.line_no  = line_no;
    track_node.size = size;
    track_node.address  = address;
    track_node.counter  = 1;

    long index = my_list_push(&track.list, (char*) &track_node);

    hash_table_bucket_s hash_entry = 
        {(void*)address, 0, (void*)index, 0, 1};
    hash_table_add(&track.hash, hash_entry);
}

void memory_track_update(void * address
    , void *prev_addr , long size
    , char *file_name , long line_no , malloc_enum type)
{
    hash_table_bucket_s entry   = 
        hash_table_get(track.hash, (void*) prev_addr, 0);
    
    if(prev_addr == NULL || entry.is_occupied != TRUE) {
        printf("Could not update store for address %p, failed!\n"
            , address);
        return;
    }

    size_t index   = (unsigned long)entry.value;
    malloc_node_s *list_i   = 
        (malloc_node_s*)my_list_get(track.list, index);
    

    list_i->type    = type;
    list_i->prev_address    = prev_addr;
    list_i->address = address;
    list_i->file_name   = file_name;
    list_i->line_no = line_no;
    list_i->counter++;

    if(size != -1)
        list_i->size    = size;

    if(address != NULL) {
        // both addresses must point to the same list node.
        hash_table_bucket_s hash_entry = 
            {(void*)address, 0, (void*)index, 0, 0};
        hash_table_add(&track.hash, hash_entry);
    }
}

void *m_malloc(size_t size, char *file_name, long line_no)
{
    void *memory    = malloc(size);
    if(memory == NULL)
        memory_exit(MALLOC_FAILED);

    memory_track_add(memory, size, file_name, line_no);

    return(memory);
}

void *m_calloc(size_t size, char *file_name, long line_no)
{
    void *memory    = calloc(size, 1);
    if(memory == NULL)
        memory_exit(CALLOC_FAILED);

    memory_track_add(memory, size, file_name, line_no);

    return(memory);
}

void *m_realloc(void *address, long size, char *file_name, long line_no)
{
    void *memory    = realloc(address, size);

    memory_track_update(memory, address
        , size, file_name, line_no, MEMORY_ALLOC_REALLOC);

    return(memory);
}

void m_free(void *address, char *file_name, long line_no)
{
    if(address){
        free(address);
        memory_track_update(NULL, address, -1, file_name
            , line_no, MEMORY_ALLOC_FREE);
    }
}

void memory_print_debug()
{
    printf("tracked memory allocations:\n\n");
    for(size_t i=0; i<track.list.index; ++i){
        malloc_node_s j = (*(malloc_node_s*)my_list_get(track.list, i));
        printf("address: %p\n"
                    "previous address: %p\n"
                    "(de)allocating file name: %s\n"
                    "(de)allocating file line number: %ld\n"
                    "request type: %d\n"
                    "requested size: %ld\n"
                    "counter: %ld\n"
                    , j.address, j.prev_address
                    , j.file_name, j.line_no
                    , j.type, j.size, j.counter);

        printf("*** preview ***\n%.*s\n*** ***\n\n"
                    , (int)(j.size > 100 ? 100: j.size)   
                    , (char*) j.address);
    }
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