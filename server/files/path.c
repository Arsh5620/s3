#include <string.h>
#include "path.h"
#include "file.h"

my_list_s
path_lex (string_s path, boolean *is_absolute)
{
    my_list_s path_tree = my_list_new (8, sizeof (file_path_s));

    char *index_p = path.address, *length_p = (path.address + path.length);

    if (length_p > index_p)
    {
        if (*index_p == '/')
        {
            *is_absolute = TRUE;
        }
    }

    while (index_p < length_p)
    {
        char *new_index_p = strchr (index_p, '/');

        long diff = new_index_p - index_p;
        file_path_node_s dir = {
          .is_file = FALSE,
          .buffer = path.address,
          .index = (ulong) (index_p - path.address),
          .length = diff};

        if (new_index_p == NULL)
        {
            dir.is_file = TRUE;
            dir.length = (ulong) (length_p - index_p);
        }

        if (diff > 0 || dir.is_file)
        {
            my_list_push (&path_tree, (char *) &dir);
        }

        if (new_index_p == NULL)
        {
            break;
        }

        index_p = new_index_p + 1;
    }
    return (path_tree);
}

file_path_s
path_parse (string_s path)
{
    boolean is_absolute = FALSE;
    my_list_s path_list = path_lex (path, &is_absolute);
    my_list_s final_path = my_list_new (10, sizeof (file_path_node_s));

    for (ulong i = 0; i < path_list.count; ++i)
    {
        file_path_node_s *node = (file_path_node_s *) my_list_get (path_list, i);
        if (node->length == 1 && *(node->buffer + node->index) == '.')
        {
            // ignore and not add this to the final list of paths.
            continue;
        }
        else if (node->length == 2 && memcmp (node->buffer + node->index, "..", 2) == 0)
        {
            // if ".." in path, it means go to the parent directory
            // we ignore requests to go beyond the string given to us
            if (final_path.count > 0)
            {
                my_list_remove (&final_path, final_path.count - 1);
            }
        }
        else
        {
            // everything else is good, perfect world!
            my_list_push (&final_path, (char *) node);
        }
    }

    my_list_free (path_list);
    file_path_s result;
    result.path_list = final_path;
    result.is_absolute = is_absolute;
    return (result);
}

string_s
path_construct (my_list_s path_list)
{
    char *buffer = m_malloc (513);
    ulong index = 0;
    for (size_t i = 0; i < path_list.count; i++)
    {
        file_path_node_s path = *(file_path_node_s *) my_list_get (path_list, i);

        if (index + path.length + 1 > 512)
        {
            m_free (buffer);
            return ((string_s){0});
        }

        memcpy (buffer + index, path.buffer + path.index, path.length);
        index += path.length;
        if (path.is_file == FALSE)
        {
            *(buffer + index) = '/';
            index++;
        }
    }
    *(buffer + index) = 0;

    char *new_buffer = m_realloc (buffer, index + 1);
    if (new_buffer != NULL)
    {
        buffer = new_buffer;
    }
    string_s file = {0};
    file.address = buffer;
    file.length = index;
    file.max_length = index + 1;
    return (file);
}

int
path_mkdir_recursive (char *path)
{
    long index = 0;
    char *next = 0;
    boolean success = TRUE;
    while (next = strchr (path + index, '/'), next != NULL)
    {
        long length = next - path;
        string_s p1 = string_new_copy (path, length);
        if (file_dir_mkine (p1.address) != FILE_DIR_EXISTS)
        {
            success = FALSE;
            break;
        }
        string_free (p1);
        index = length + 1;
    }
    return (success ? SUCCESS : FAILED);
}

void
path_free (file_path_s path)
{
    my_list_free (path.path_list);
}