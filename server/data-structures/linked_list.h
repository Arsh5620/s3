#ifndef LINKED_LIST_INCLUDE_GAURD
#define LINKED_LIST_INCLUDE_GAURD

typedef struct linked_list_node {
	char *child;
	char *parent;
	char *data;
	long length;
} linked_list_node_s;

typedef struct linked_list {
	struct linked_list_node *root;
	struct linked_list_node *last;
	long counter;
} linked_list_s;

void linked_list_push(linked_list_s *list, char *data, long length);
void linked_list_remove(linked_list_s *root, long index);
void linked_list_free(linked_list_s *root);

#endif //LINKED_LIST_INCLUDE_GAURD