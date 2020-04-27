# ifndef LIST_INCLUDE_GAURD
# define LIST_INCLUDE_GAURD
#include <stdio.h>
#include <stdlib.h>

#define LIST_INCREASE 128

typedef struct array_list {
	size_t count;	/* current count of the elements */
	size_t size;	/* max number of elements allowed */
	int block;		/* size of a single element in bytes */
	char *address;	/* memory address for the list */
} my_list_s;

void my_list_free(my_list_s list) ;
char *my_list_get(my_list_s list, size_t index);
void my_list_remove(my_list_s *list, size_t index);
size_t my_list_push(my_list_s* list, char* memory);
my_list_s my_list_new(size_t size , size_t entry_length);

#endif