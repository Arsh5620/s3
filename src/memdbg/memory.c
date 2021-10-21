#include "memory.h"
#include "data/hash_table.h"
#include "data/linked_list.h"
#include "general/common.h"
#include "general/messages.h"
#include "logging/logs.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG

// definitions

typedef enum {
    MEMORY_MALLOC,
    MEMORY_CALLOC,
    MEMORY_REALLOC,
    MEMORY_FREE,
} memory_op_type_enum;

typedef struct {
    char *file_name;
    long size;
    long line_no;
    memory_op_type_enum type;
} memory_op_s;

typedef struct {
    void *address;
    void *new_addr;
    memory_op_type_enum latest;
    linked_list_s allocations;
} memory_allocation_s;

typedef struct {
    hash_table_s table;
    int initialized;
} memory_s;

void
memory_free ();
char *
memory_type (memory_op_type_enum op_type);
void
memory_log (memory_allocation_s *alloc, memory_op_s operation);
long
memory_get_size (memory_allocation_s *alloc);

// implementations

static memory_s allocations = {0};
static char *mem_types[] = {"malloc", "calloc", "realloc", "free"};

void
memory_add (void *address, long size, char *file_name, long line_no)
{
    if (allocations.initialized == FALSE) {
        allocations.table = hash_table_alloc (512, 0.85f, 2, 0);
        allocations.initialized = TRUE;
    }

    memory_allocation_s *allocation = calloc (1, sizeof (*allocation));
    allocation->address = address;
    allocation->latest = MEMORY_MALLOC;

    memory_op_s operation = {.type = MEMORY_MALLOC, .file_name = file_name, .line_no = line_no, .size = size};

    allocation->allocations = linked_list_new (sizeof (operation));
    linked_list_add (&allocation->allocations, &operation);

    hash_table_add (&allocations.table, (char *) &address, sizeof (address), (char *) allocation, sizeof (*allocation));
    memory_log (allocation, operation);
}

void
memory_update (void *address, void *new_addr, long size, char *file_name, long line_no, memory_op_type_enum type)
{
    hash_table_bucket_s entry = hash_table_get (allocations.table, (char *) &address, sizeof (address));

    if (entry.occupied == FALSE) {
        char *error = MEMORY_ALLOCATION_NOENTRY;
        logs_put (LOG_OUT_LOGS,
                  LOG_LEVEL_DEBUG,
                  MEMORY_ALLOCATION_ERROR,
                  address,
                  new_addr,
                  memory_type (type),
                  size,
                  file_name,
                  line_no,
                  error);
        return;
    }

    memory_allocation_s *allocation = (memory_allocation_s *) entry.value;

    if (type == MEMORY_FREE && size == 0) {
        size = memory_get_size (allocation);
    }

    allocation->latest = type;
    memory_op_s operation = {.type = type, .file_name = file_name, .line_no = line_no, .size = size};

    linked_list_add (&allocation->allocations, (char *) &operation);

    if (type == MEMORY_FREE) {
        hash_table_remove (&allocations.table, (char *) &address, sizeof address);
    } else if (type == MEMORY_REALLOC && new_addr != NULL && address != new_addr) {
        memory_add (new_addr, size, file_name, line_no);
        allocation->new_addr = new_addr;

        memory_op_s operation2 = {.type = MEMORY_FREE, .file_name = file_name, .line_no = line_no, .size = size};
        linked_list_add (&allocation->allocations, (char *) &operation2);
    }

    memory_log (allocation, operation);
}

long
memory_get_size (memory_allocation_s *allocation)
{
    memory_op_s *operation = (memory_op_s *) allocation->allocations.last->data;
    return operation->size;
}

void *
__malloc (long size, char *file_name, long line_no)
{
    void *memory = malloc (size);
    assert (memory != NULL);
    memory_add (memory, size, file_name, line_no);
    return (memory);
}

void *
__calloc (long size, char *file_name, long line_no)
{
    void *memory = calloc (size, 1);
    assert (memory != NULL);
    memory_add (memory, size, file_name, line_no);
    return (memory);
}

void *
__realloc (void *address, long size, char *file_name, long line_no)
{
    void *memory = realloc (address, size);
    size = memory == NULL ? -1 : size;
    memory_update (address, memory, size, file_name, line_no, MEMORY_REALLOC);
    return (memory);
}

void
__free (void *address, char *file_name, long line_no)
{
    if (address) {
        free (address);
        memory_update (address, address, 0, file_name, line_no, MEMORY_FREE);
    }
}

char *
memory_type (memory_op_type_enum type)
{
    return mem_types[type];
}

void
memory_log (memory_allocation_s *alloc, memory_op_s operation)
{
    logs_put (LOG_OUT_LOGS,
              LOG_LEVEL_DEBUG,
              MEMORY_ALLOCATION_LOG,
              alloc->address,
              alloc->new_addr,
              memory_type (operation.type),
              operation.size,
              operation.file_name,
              operation.line_no);
}

// function to release all the memory used by the structures
void
memory_cleanup ()
{
    for (int i = 0; i < allocations.table.capacity; ++i) {
        hash_table_bucket_s bucket = hash_table_get_n (allocations.table, i);
        if (bucket.occupied != HASH_OCCUPIED) {
            continue;
        }

        memory_allocation_s *alloc = (memory_allocation_s *) bucket.value;
        linked_list_free (alloc->allocations);
        free (alloc);
    }

    hash_table_free (allocations.table);
}

void
memory_debug_print_lost ()
{
    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, MEMORY_UNFREE_LOG_TITLE);
    for (int i = 0; i < allocations.table.capacity; ++i) {
        hash_table_bucket_s bucket = hash_table_get_n (allocations.table, i);
        if (bucket.occupied != HASH_OCCUPIED) {
            continue;
        }

        memory_allocation_s *alloc = (memory_allocation_s *) bucket.value;

        logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, MEMORY_UNFREE_LOG, alloc->address, alloc->new_addr, memory_type (alloc->latest));

        linked_list_s update_list = alloc->allocations;
        linked_list_node_s *updates = update_list.first;

        if (updates != NULL) {
            logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, MEMORY_UNFREE_LOG_UPDATES_TITLE);
        }

        while (updates != NULL) {
            memory_op_s *update = (memory_op_s *) updates->data;

            logs_put (LOG_OUT_LOGS,
                      LOG_LEVEL_DEBUG,
                      MEMORY_UNFREE_LOG_UPDATES,
                      update->file_name,
                      update->line_no,
                      update->size,
                      memory_type (update->type));

            updates = updates->next;
        }
    }
}

#else

void inline __free (void *address, char *file_name, long line_no) { free (address); }

void inline *
__malloc (long size, char *file_name, long line_no)
{
    return malloc (size);
}

void inline *
__realloc (void *address, long size, char *file_name, long line_no)
{
    return realloc (address, size);
}
void inline *
__calloc (long size, char *file_name, long line_no)
{
    return calloc (size, 1);
}

void
memory_debug_print_lost ()
{
}

#endif // DEBUG