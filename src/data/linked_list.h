#ifndef SRC_DATA_LINKED_LIST_
#define SRC_DATA_LINKED_LIST_

typedef struct linked_list_node {
    struct linked_list_node *next;
    struct linked_list_node *prev;
    void *data;
} linked_list_node_s;

typedef struct{
    struct linked_list_node *first;
    struct linked_list_node *last;
    long counter;
    long item_size;
} linked_list_s;

linked_list_s
linked_list_new (long item_size);
void
linked_list_free (linked_list_s list);

void
linked_list_add (linked_list_s *list, void *data);
void
linked_list_remove (linked_list_s *list, long index);

#endif // SRC_DATA_LINKED_LIST_