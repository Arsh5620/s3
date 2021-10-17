#ifndef DATA_LINKED_LIST_
#define DATA_LINKED_LIST_

typedef struct linked_list_node {
    struct linked_list_node *child;
    struct linked_list_node *parent;
    char *data;
    long length;
} linked_list_node_s;

typedef struct linked_list {
    struct linked_list_node *root;
    struct linked_list_node *last;
    long counter;
} linked_list_s;

void
linked_list_push (linked_list_s *list, char *data, long length);
void
linked_list_remove (linked_list_s *root, long index);
void
linked_list_free (linked_list_s *root);

#endif // DATA_LINKED_LIST_