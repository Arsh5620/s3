#include <malloc.h>
#include <string.h>
#include "linked_list.h"

void
linked_list_push (linked_list_s *list, char *data, long length)
{
    linked_list_node_s *node = calloc (1, sizeof (linked_list_node_s));
    if (list->root == NULL)
    {
        list->root = node;
    }
    node->parent = (char *) list->last;
    if (list->last != NULL)
        list->last->child = (char *) node;

    if (length)
    {
        node->data = malloc (length);
        memcpy (node->data, data, length);
    }
    node->length = length;
    list->last = node;
}

void
linked_list_remove (linked_list_s *list, long index)
{
    linked_list_node_s *node = list->root;
    for (long i = 0; i < index && node != NULL; ++i)
    {
        node = (linked_list_node_s *) node->child;
    }
    if (node != NULL && node->parent != NULL)
    {
        node->parent = node->child;
    }
    if (node->data != NULL)
        free (node->data);
    if (node != NULL)
        free (node);
}

void
linked_list_free (linked_list_s *list)
{
    linked_list_node_s *node = list->root;
    while (node != NULL)
    {
        if (node->data != NULL)
            free (node->data);

        linked_list_node_s *n1 = node;
        node = (linked_list_node_s *) node->child;
        free (n1);
    }
}