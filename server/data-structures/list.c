#include <memory.h>
#include <assert.h>
#include "list.h"

my_list_s my_list_new(size_t count , size_t block)
{
	my_list_s list = {0};

	size_t size	= (count * block);
	list.block	= block;
	list.size	= count;
	list.address	= calloc(size, 1);

	assert(list.address != (void*)0);
	return(list);
}

void my_list_grow(my_list_s *list) 
{
	list->size	+= LIST_INCREASE;
	size_t size = list->size * list->block;

	size_t copy = (list->count * list->block);
	char *address	= realloc(list->address, size);

	if (NULL == address) {
		address = calloc(size, 1);
		assert(address != NULL);

		memcpy(address, list->address, copy);
		free(list->address); /* free the original memory space after copy */
	} else {
		/* zero the memory after list->count elements */
		memset(address + copy, 0, size - copy);
	}
	list->address = address;
}

size_t my_list_push(my_list_s *list, char *entry)
{
	if (list->count == list->size) 
	{
		my_list_grow(list);
	}

	size_t size	= list->count * list->block;
	memcpy(list->address + size, entry, list->block);

	list->count++;
	return(list->count - 1);
}

char *my_list_get(my_list_s list, size_t index)
{
	if (index >= 0 && index <  list.count) {
		size_t size	= list.block * index;
		return (list.address + size);
	} else  {
		return (NULL);
	}
}

void my_list_remove(my_list_s *list, size_t index)
{
	assert(index >= 0 && index < list->count);

	size_t move	= (list->count - index - 1) * list->block;
	size_t add	= (index * list->block);
	memmove(list->address + add
		, list->address + add + list->block, move);
	list->count--;
}

void my_list_free(my_list_s list) 
{
	if (list.address)
		free(list.address);
}
