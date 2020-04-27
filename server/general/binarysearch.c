#include "binarysearch.h"
#include <string.h>

/* 
 * @ returns the index of the element in the given array by
 * performing binary search on the memory with "compare"
 */
size_t binary_search(void *memory, size_t block_length
	, size_t count, char *element, size_t length
	, size_t (*compare)(void *, char *, size_t))
{
	size_t min = 0;
	size_t max = count;

	while (min < max)
	{
		size_t split    = (max + min) / 2;

		char *memory_index	= (memory + (split * block_length));
		signed int result	= compare(memory_index, element, length);
			
		if (result < 0)
		{
			max = split;
		}
		else if (result > 0)
		{
			min = split + 1;
		}
		else 
		{
			return(split);
		}
	}

	return (BINARYSEARCH_NORESULT);
}