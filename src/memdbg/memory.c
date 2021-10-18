#include "memory.h"
#include "general/common.h"
#include "general/messages.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG

static malloc_s allocations = {0};

void
memory_track_add (void *address, long size, char *file_name, long line_no)
{
    if (allocations.is_init == FALSE) {
        allocations.list = my_list_new (MEMORY_TABLE_SIZE, sizeof (malloc_node_s));
        allocations.hash = hash_table_init_dbg (MEMORY_TABLE_SIZE, 0, 0);
        allocations.is_init = TRUE;
    }

    malloc_node_s track_node = {.address = address, .last_update = MEMORY_ALLOC_MALLOC};

    malloc_update_s node = {.type = MEMORY_ALLOC_MALLOC, .file_name = file_name, .line_no = line_no, .size = size};

    linked_list_push (&track_node.updates, (char *) &node, sizeof (node));
    long index = my_list_push (&allocations.list, (char *) &track_node);

    hash_table_bucket_s hash_entry = {
      .key.address = (void *) address, .value.number = index, .key_len = 0, .value_len = 0, .is_occupied = 0};

    hash_table_add (&allocations.hash, hash_entry);
    memory_log_handle (MEMORY_ALLOC_MALLOC, &track_node, &node);
}

/*
 * note: realloc with different dest address is implemented as subsequent
 * calls to malloc and free respectively. no change for realloc with
 * same dest address.
 */
void
memory_track_update (void *address, void *new_addr, long size, char *file_name, long line_no, malloc_enum type)
{
    hash_input_u key = {.address = (void *) address};
    hash_table_bucket_s entry = hash_table_get (allocations.hash, key, 0);

    if (entry.is_occupied != TRUE) {
        char *errorm = MEMORY_ALLOCATION_NOENTRY;
        logs_put (LOG_OUT_LOGS,
                  LOG_LEVEL_DEBUG,
                  MEMORY_ALLOCATION_ERROR,
                  address,
                  new_addr,
                  memory_log_gettype (type),
                  size,
                  file_name,
                  line_no,
                  errorm);
        return;
    }

    malloc_node_s *allocation = (malloc_node_s *) my_list_get (allocations.list, entry.value.number);

    if (type == MEMORY_ALLOC_FREE && size == 0) {
        size = memory_get_allocation_size (allocation);
    }

    allocation->last_update = type;

    malloc_update_s node = {.type = type, .file_name = file_name, .line_no = line_no, .size = size};

    linked_list_push (&allocation->updates, (char *) &node, sizeof (node));

    if (type == MEMORY_ALLOC_FREE) {
        hash_input_u rmindex = (hash_input_u){.address = address};
        hash_table_remove (&allocations.hash, rmindex, 0);
    } else if (type == MEMORY_ALLOC_REALLOC && new_addr != NULL && address != new_addr) {
        memory_track_add (new_addr, size, file_name, line_no);
        allocation->new_addr = new_addr;

        node.type = MEMORY_ALLOC_FREE;
        node.file_name = file_name;
        node.line_no = line_no;
        node.size = size;

        linked_list_push (&allocation->updates, (char *) &node, sizeof (node));
    }

    memory_log_handle (type, allocation, &node);
}

long
memory_get_allocation_size (malloc_node_s *node)
{
    malloc_update_s *update = (malloc_update_s *) ((linked_list_node_s *) (node->updates.last))->data;
    return (update->size);
}

void *
__malloc (size_t size, char *file_name, long line_no)
{
    void *memory = malloc (size);
    assert (memory != NULL);
    memory_track_add (memory, size, file_name, line_no);
    return (memory);
}

void *
__calloc (size_t size, char *file_name, long line_no)
{
    void *memory = calloc (size, 1);
    assert (memory != NULL);
    memory_track_add (memory, size, file_name, line_no);
    return (memory);
}

void *
__realloc (void *address, long size, char *file_name, long line_no)
{
    void *memory = realloc (address, size);
    size = memory == NULL ? -1 : size;
    memory_track_update (address, memory, size, file_name, line_no, MEMORY_ALLOC_REALLOC);
    return (memory);
}

void
__free (void *address, char *file_name, long line_no)
{
    if (address) {
        free (address);
        memory_track_update (address, address, 0, file_name, line_no, MEMORY_ALLOC_FREE);
    }
}

char *
memory_log_gettype (malloc_enum i)
{
    static char *mem_types[] = {"malloc", "calloc", "realloc", "free"};
    return mem_types[i];
}

void
memory_log_handle (malloc_enum type, malloc_node_s *node, malloc_update_s *update)
{
#ifdef MEMDBG
    logs_put (LOG_OUT_LOGS,
              LOG_LEVEL_DEBUG,
              MEMORY_ALLOCATION_LOG,
              node->address,
              node->new_addr,
              memory_log_gettype (update->type),
              update->size,
              update->file_name,
              update->line_no);
#endif
}

// function to release all the memory used by the structures
void
memory_cleanup ()
{
    hash_table_free (allocations.hash);
    for (long i = 0; i < allocations.list.count; ++i) {
        malloc_node_s *node = (malloc_node_s *) my_list_get (allocations.list, i);
        linked_list_free (&node->updates);
    }
    my_list_free (allocations.list);
}

void
memory_debug_print_unfreed ()
{
    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, MEMORY_UNFREE_LOG_TITLE);
    for (size_t i = 0; i < allocations.list.count; i++) {
        malloc_node_s *node = (malloc_node_s *) my_list_get (allocations.list, i);
        boolean is_alloc = hash_table_get (allocations.hash, (hash_input_u){.address = node->address}, 0).is_occupied;
        if (is_alloc) {
            logs_put (LOG_OUT_LOGS,
                      LOG_LEVEL_DEBUG,
                      MEMORY_UNFREE_LOG,
                      node->address,
                      node->new_addr,
                      memory_log_gettype (node->last_update));

            linked_list_s update_list = node->updates;
            linked_list_node_s *node = update_list.root;

            if (node != NULL) {
                logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, MEMORY_UNFREE_LOG_UPDATES_TITLE);
            }

            while (node != NULL) {
                malloc_update_s *update = (malloc_update_s *) node->data;

                logs_put (LOG_OUT_LOGS,
                          LOG_LEVEL_DEBUG,
                          MEMORY_UNFREE_LOG_UPDATES,
                          update->file_name,
                          update->line_no,
                          update->size,
                          memory_log_gettype (update->type));

                node = node->child;
            }

            logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, EMPTY_STRING);
        }
    }
}

#else

void inline __free (void *address, char *file_name, long line_no) { free (address); }

void inline *
__malloc (size_t size, char *file_name, long line_no)
{
    return malloc (size);
}

void inline *
__realloc (void *address, long size, char *file_name, long line_no)
{
    return realloc (address, size);
}
void inline *
__calloc (size_t size, char *file_name, long line_no)
{
    return calloc (size, 1);
}

#endif // DEBUG