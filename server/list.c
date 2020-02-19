#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "list.h"
#include "memory.h"

array_list_s my_list_new(size_t size , size_t entry_length)
{
    array_list_s array_list = {0};

    size_t memory_needed    = (size * entry_length);
    array_list.entry_length = entry_length;
    array_list.length   = size;
    array_list.memory   = calloc(memory_needed, 1);

    if(array_list.memory == (void*)0){
        printf("Allocating memory failed with malloc,"
            " file: %s, lineno: %d\n", __FILE__, __LINE__);
    }
    return(array_list);
}

void my_list_expand_memory(array_list_s *list) 
{
    list->length    += DEFAULT_MEMORY_INCREASE;
    size_t new_size = list->length * list->entry_length;

	void *memory = 0;
	size_t copy_size = (list->index * list->entry_length);
	if ((memory = realloc(list->memory, new_size)) == 0) {
		memory = calloc(new_size, 1);
		memcpy(memory, list->memory, copy_size);
		free(list->memory);
	}
	else 
        memset(((char*)memory + copy_size), 0, new_size - copy_size);
    list->memory = memory;
}

size_t my_list_push(array_list_s *list, char *entry)
{
    if(list->index == list->length)
        my_list_expand_memory(list);

    memcpy((list->memory + (list->index * list->entry_length))
        , entry, list->entry_length);
    return(list->index++);
}

void *my_list_get(array_list_s list, size_t index)
{
    if(index <= list.length)
        return (list.memory + (index * list.entry_length));
    else 
        return (NULL);
}

void my_list_delete(array_list_s *list) 
{
    if(list->memory)
        free(list->memory);
}
