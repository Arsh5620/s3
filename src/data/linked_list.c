#include "linked_list.h"

#include <malloc.h>
#include <string.h>

linked_list_s
linked_list_new (long item_size)
{
    linked_list_s list = {0};
    list.item_size = item_size;
    return list;
}

void
linked_list_add (linked_list_s *list, void *data)
{
    linked_list_node_s *node = malloc (sizeof (linked_list_node_s) + list->item_size);

    if (list->first == NULL) {
        list->first = node;
    }

    node->next = NULL;
    node->prev = list->last;

    if (list->last != NULL) {
        list->last->next = node;
    }

    node->data = ((void *) node) + sizeof (linked_list_node_s);
    memcpy (node->data, data, list->item_size);

    list->last = node;
}

void
linked_list_remove (linked_list_s *list, long index)
{
    linked_list_node_s *node = list->first;

    for (long i = 0; i < index && node != NULL; ++i) {
        node = node->next;
    }

    if (node != NULL) {
        linked_list_node_s *prev = node->prev;
        linked_list_node_s *next = node->next;

        prev->next = next;
        next->prev = prev;
    }

    if (node != NULL) {
        free (node);
    }
}

void
linked_list_free (linked_list_s list)
{
    linked_list_node_s *node = list.first;

    while (node != NULL) {
        linked_list_node_s *temp = node;
        node = (linked_list_node_s *) node->next;
        free (temp);
    }
}