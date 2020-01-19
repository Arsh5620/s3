typedef struct array_list {
    int index;
    int index_computed;
    int raw_size;
    int length;
    int entry_size;

    void *memory; 
} array_list_s;

#define DEFAULT_MEMORY_INCREASE 128

array_list_s list_new(int, int);
void *list_get(array_list_s, int);
unsigned int list_push(array_list_s *, void *);
