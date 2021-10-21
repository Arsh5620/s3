#include "path.h"
#include "file.h"
#include "general/string.h"
#include "memdbg/memory.h"

#include <assert.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const char *
path_get_home ()
{
    const char *homedir;
    if ((homedir = getenv ("HOME")) == NULL) {
        homedir = getpwuid (getuid ())->pw_dir;
    }
    return homedir;
}

void *
path_alloc_memory (void *src, int *current_size, int index, int size)
{
    if (*current_size <= index + size) {
        *current_size = *current_size * 2 + size;

        char *memory = m_realloc (src, *current_size);
        if (memory == NULL) {
            char *alloc = m_malloc (*current_size);
            assert (alloc != NULL);

            memcpy (alloc, src, index);
            alloc[index] = 0;
            memory = alloc;
        }
        return memory;
    }

    return src;
}

char *
path_validate_and_expand (char *path)
{
    int current_path_size = 256;
    int current_positions_size = 256;

    char *new_path = m_malloc (current_path_size);
    short *positions = m_malloc (current_positions_size); // For 128 short position entries
    assert (new_path != NULL);
    assert (positions != NULL);

    short index = 0;
    short position_count = 0;
    char *path_ptr = path;

    if (*path == '/') {
        *new_path = *path_ptr; // new_path is also absolute
        path_ptr++;
        index++;
    } else if (*path == '~') {
        const char *home = path_get_home ();
        int home_length = strlen (home);
        new_path = path_alloc_memory (new_path, &current_path_size, index, home_length + 1);

        memcpy (new_path, home, home_length);
        new_path[home_length] = '/';

        path_ptr++;
        index += home_length + 1;
    }

    positions[position_count++] = index;

    do {
        char *next = strchr (path_ptr, '/');
        if (next == NULL) {
            break;
        }

        short length = next - path_ptr;

        if (length == 1 && path_ptr[0] == '.') {
            // Ignore this
        } else if (length == 2 && path_ptr[0] == '.' && path_ptr[1] == '.') {
            if (position_count > 0) {
                --position_count;
                index = positions[position_count];
            } else {
                // Ignore
            }
        } else if (length > 0) {
            new_path = path_alloc_memory (new_path, &current_path_size, index, length);
            positions = (short *) path_alloc_memory (
              (char *) positions, &current_positions_size, position_count * sizeof *positions, sizeof (*positions));

            memcpy (new_path + index, path_ptr, length);
            new_path[index + length] = '/';
            positions[position_count++] = index;
            index += length + 1;
        }

        path_ptr = next + 1;
    } while (path_ptr != (void *) 1);

    int length_file_name = strlen (path_ptr);
    if (length_file_name > 0) {
        new_path = path_alloc_memory (new_path, &current_path_size, index, length_file_name);

        memcpy (new_path + index, path_ptr, length_file_name);
        index += length_file_name;
    }

    new_path[index] = 0;
    new_path = m_realloc (new_path, index + 1);
    assert (new_path != NULL);

    return new_path;
}

char *
path_concat_and_validate (char *path_first, char *path_second, short path_first_length, short path_second_length)
{
    long length;
    char *path = string_sprintf ("%.*s/%.*s", &length, path_first_length, path_first, path_second_length, path_second);
    return path_validate_and_expand (path);
}

/**
 * this function requires that all directory names end will a forward slash /
 * otherwise the last part of the path is interpreted as a file instead
 */
int
path_mkdir_recursive (char *path)
{
    char *full_path = path_validate_and_expand (path);

    long index = 0;
    char *next = 0;
    boolean success = TRUE;
    while (next = strchr (full_path + index, '/'), next != NULL) {
        long length = next - full_path;
        string_s p1 = string_new_copy (full_path, length);
        if (length > 0 && file_dir_create (p1.address) != FILE_DIR_EXISTS) {
            success = FALSE;
            break;
        }
        string_free (p1);
        index = length + 1;
    }

    m_free (full_path);
    return (success ? SUCCESS : FAILED);
}