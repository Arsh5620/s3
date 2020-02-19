# ifndef LIST_INCLUDE_GAURD
# define LIST_INCLUDE_GAURD
#include <stdlib.h>

#define DEFAULT_MEMORY_INCREASE 128

typedef struct array_list {
    size_t index;
    size_t length;
    short entry_length;
    char *memory;
} array_list_s;

array_list_s my_list_new(size_t size , size_t entry_length);
void *my_list_get(array_list_s list, size_t index);
size_t my_list_push(array_list_s* list, char* memory);
void my_list_delete(array_list_s *list);

#endif