#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "./file.h"
#include "./path.h"
#include "../memdbg/memory.h"
#include "../general/string.h"

// make a directory if not exists, returns FILE_DIR_EXISTS if the directory
// is either created or already exists, and returns error otherwise.
int
file_dir_mkine (char *dir_name)
{
    DIR *dir = opendir (dir_name);
    int error = errno;

    if (dir != NULL)
    {
        closedir (dir);
        return (FILE_DIR_EXISTS);
    }
    else if (error == ENOENT)
    {
        error = mkdir (dir_name, S_IRWXU | S_IRGRP | S_IXGRP);
        if (error != 0)
        {
            return FILE_DIR_CREATE_FAILED;
        }
        else
        {
            return FILE_DIR_EXISTS;
        }
    }
    return (FILE_DIR_ERROR_OPEN);
}

FILE *
file_open (string_s name, char *mode)
{
    if (name.length > FILE_NAME_LENGTH)
    {
        return (NULL);
    }

    string_s name_null = string_new_copy (name.address, name.length);
    FILE *file = fopen (name_null.address, mode);
    string_free (name_null);

    return (file);
}

int
file_delete (char *filename)
{
    return (unlink (filename));
}

int
file_download (
  FILE *file,
  network_s *network,
  ulong size,
  file_sha1 *hash,
  void (*sha1_hash) (file_sha1 *, network_data_s, boolean))
{
    ulong read_required = 0, index = 0;
    do
    {
        read_required = size - index;

        if (read_required > FILE_BUFFER_LENGTH)
        {
            read_required = FILE_BUFFER_LENGTH;
        }

        if (read_required == 0)
        {
            break;
        }

        network_data_s data = network_read_stream (network, read_required);

        if (data.error_code)
        {
            network_data_free (data);
            return (FILE_NETWORK_ERROR);
        }

        int written = fwrite (data.data_address, 1, data.data_length, file);

        if (written != data.data_length)
        {
            return (FILE_WRITE_ERROR);
        }

        sha1_hash (hash, data, FALSE);
        network_data_free (data);
        index += data.data_length;
    } while (index < size);

    sha1_hash (hash, (network_data_s){0}, TRUE);

    fflush (file);
    return (FILE_SUCCESS);
}

struct stat
file_stat (FILE *file)
{
    struct stat file_stat = {0};
    fstat (file->_fileno, &file_stat);
    return (file_stat);
}

// this function initializes file_reader_s so that files can be
// read as stream without having to read the entire file into memory.
file_reader_s
file_reader_init (FILE *file)
{
    file_reader_s reader = {0};
    if (file == NULL)
    {
        return (reader);
    }

    reader.file = file;
    reader.maxlength = FILE_BUFFER_LENGTH;
    reader.buffer = m_malloc (FILE_BUFFER_LENGTH, MEMORY_FILE_LINE);
    reader.stats = file_stat (file);
    return (reader);
}

void
file_reader_close (file_reader_s *reader)
{
    if (reader->buffer)
    {
        free (reader->buffer);
    }
}

// fills the buffer at index location with size bytes
int
file_reader_next (file_reader_s *reader, long index, long size)
{
    if (reader->eof)
    {
        reader->readlength = -1;
        return (FILE_ERROR);
    }

    int read = fread (reader->buffer + index, 1, size, reader->file);

    reader->eof = feof (reader->file);
    reader->readlength = read;

    if (read != size && reader->eof == FALSE)
    {
        return (FILE_READ_ERROR);
    }
    return (FILE_SUCCESS);
}

string_s
file_path_concat (string_s path1, string_s path2)
{
    long length;
    char *path = string_sprintf (
      "%.*s/%.*s", &length, path1.length, path1.address, path2.length, path2.address);

    string_s path_s;
    path_s.address = path;
    path_s.length = length;

    file_path_s normalized_path = path_parse (path_s);
    string_s result = path_construct (normalized_path.path_list);
    m_free (path, MEMORY_FILE_LINE);
    path_free (normalized_path);

    return (result);
}

int
file_rename (string_s dest, string_s src)
{
    return (rename (src.address, dest.address));
}

int
file_append (char *dest, char *src, ulong index, ulong size)
{
    ulong maxread = FILE_BUFFER_LENGTH;
    char *buffer = m_malloc (maxread, MEMORY_FILE_LINE);
    ulong written = 0;

    FILE *src_file = fopen (src, FILE_MODE_READBINARY);
    FILE *dest_file = fopen (dest, FILE_MODE_WRITEEXISTING);

    if (src_file == NULL || dest_file == NULL)
    {
        return (FILE_ERROR_OPEN);
    }

    if (fseek (dest_file, index, SEEK_SET))
    {
        return (FILE_ERROR_SEEK);
    }

    while (written < size)
    {
        ulong read_required = (size - written) > maxread ? maxread : (size - written);

        ulong read_len = fread (buffer, 1, read_required, src_file);

        if (read_len == -1 || read_len != read_required)
        {
            return (FILE_READ_ERROR);
        }

        ulong write_len = fwrite (buffer, 1, read_len, dest_file);

        if (write_len == -1 || write_len != read_len)
        {
            return (FILE_WRITE_ERROR);
        }

        written += write_len;
    }
    return (FILE_SUCCESS);
}