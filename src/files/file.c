#include "file.h"
#include "general/string.h"
#include "memdbg/memory.h"
#include "path.h"

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Make a directory if not exists, returns FILE_DIR_EXISTS if the directory
 * is either created or already exists, and returns error otherwise.
 */
file_errors_e
file_dir_create (char *dir_name)
{
    DIR *dir = opendir (dir_name);

    if (dir != NULL) {
        closedir (dir);
        return FILE_DIR_EXISTS;
    } else if (errno == ENOENT) {
        if (mkdir (dir_name, S_IRWXU | S_IRGRP | S_IXGRP)) {
            return FILE_DIR_CREATE_FAILED;
        } else {
            return FILE_DIR_EXISTS;
        }
    }

    return FILE_DIR_ERROR_OPEN;
}

struct stat
file_stat (FILE *file)
{
    struct stat file_stat = {0};
    fstat (file->_fileno, &file_stat);
    return file_stat;
}

int
file_rename (string_s dest, string_s src)
{
    return rename (src.address, dest.address);
}

int
file_append (char *dest, char *src, ulong index, ulong size)
{
    ulong maxread = FILE_BUFFER_LENGTH;
    char *buffer = m_malloc (maxread);
    ulong written = 0;

    FILE *src_file = fopen (src, FILE_MODE_READBINARY);
    FILE *dest_file = fopen (dest, FILE_MODE_WRITEEXISTING);

    if (src_file == NULL || dest_file == NULL) {
        return FILE_ERROR_OPEN;
    }

    if (fseek (dest_file, index, SEEK_SET)) {
        return FILE_ERROR_SEEK;
    }

    while (written < size) {
        ulong read_required = min (size - written, maxread);
        ulong read_len = fread (buffer, sizeof (char), read_required, src_file);

        if (read_len == -1 || read_len != read_required) {
            return (FILE_READ_ERROR);
        }

        ulong write_len = fwrite (buffer, 1, read_len, dest_file);

        if (write_len == -1 || write_len != read_len) {
            return (FILE_WRITE_ERROR);
        }

        written += write_len;
    }

    fflush (dest_file);
    return FILE_SUCCESS;
}