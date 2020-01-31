#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "list.h"
#include "defines.h"

array_list_s list_new(int default_size, int entry_size)
{
    array_list_s array_list = {0};

    array_list.raw_size     = (default_size * entry_size);
    array_list.entry_size   = entry_size;
    array_list.length       = default_size;

    array_list.memory   = calloc(array_list.raw_size, 1);

    if(array_list.memory == (void*)0){
        printf("Allocating memory failed with malloc, fun"
                "ction name: list_new, file name: list.c\n");
        exit(MALLOC_FAILED);
    }
    return(array_list);
}

void __list_expand_memory(array_list_s *list) 
{

		list->raw_size  += (list->entry_size * DEFAULT_MEMORY_INCREASE);
        list->length    += DEFAULT_MEMORY_INCREASE;
        
		void *memory = 0;
		int copy_size = list->index_computed;
		if ((memory = realloc(list->memory, list->raw_size)) == 0) {
			memory = calloc(list->raw_size, 1);
			memcpy(memory, list->memory, copy_size);
			free(list->memory);
		}
		else memset(((char*)memory + copy_size), 0, list->raw_size - copy_size);
        list->memory = memory;
}

unsigned int list_push(array_list_s *list, void *entry)
{
    if(list->index_computed == list->raw_size)
        __list_expand_memory(list);

    memcpy((list->memory + list->index_computed), entry, list->entry_size);
    list->index_computed += list->entry_size;
    return(list->index++);
}

void *list_get(array_list_s list, int index)
{
    if(index <= list.length)
        return (list.memory + (index * list.entry_size));
    else return 0;
}