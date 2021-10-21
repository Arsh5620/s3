#ifndef SRC_FILES_FILE_
#define SRC_FILES_FILE_

#include "general/string.h"
#include "networking/network.h"
#include "ssbs/list.h"

#include <stdio.h>
#include <sys/stat.h>

typedef enum file_errors_enum {
    FILE_SUCCESS,
    FILE_ERROR,
    FILE_EOF,
    FILE_DIR_EXISTS,
    FILE_DIR_CREATE_FAILED,
    FILE_DIR_ERROR_OPEN,
    FILE_ERROR_OPEN,
    FILE_ERROR_SEEK,
    FILE_NETWORK_ERROR,
    FILE_NETWORK_ASYNC_WOULBLOCK,
    FILE_WRITE_ERROR,
    FILE_READ_ERROR
} file_errors_e;

#define FILE_MODE_WRITEBINARY "wb"
#define FILE_MODE_READBINARY "rb"
#define FILE_MODE_WRITEEXISTING "rb+"
#define FILE_MODE_APPENDBINARY "ab"

#define FILE_NAME_LENGTH (256)
#define FILE_BUFFER_LENGTH (KB (16))
#define FILE_SHA1_SIZE (160 / 8)

typedef struct file_hash {
    /**
     * hash_function is called on a given data buffer to compute the
     * hash of that buffer, while hash_compute_length is the length
     * of the output of a given hashing algorithm, such as
     * hash_compute_length for sha1 is 20
     */
    void (*hash_function) (void *hash, char *hash_buffer, int hash_buffer_length);
    int hash_compute_length;

    /**
     * hash_list will store all the hashes for the given file,
     * while hash_buffer, hash_index, and hash_size is used to
     * buffer the data so that we have enough data to perform
     * the hash.
     * So, like right now we are using 1MB as the buffer size
     * , so until we have 1MB of data we will keep on buffering
     * but once we have enough data we will call the hash_function
     * and it will create a hash that we will then add to the hash_list
     * and then we will clear the buffer and start over again.
     */
    my_list_s hash_list;
    char *hash_buffer;
    ulong hash_index, hash_size;
} file_hash_s;

typedef struct {
    FILE *file;
    struct stat stats;

    char *buffer;
    long read;
    long buffer_length;
    boolean eof;
} file_reader_s;

file_errors_e
file_dir_create (char *dir_name);
int
file_delete (char *file);
int
file_rename (string_s dest, string_s src);
struct stat
file_stat (FILE *file);

int
file_append (char *dest, char *src, ulong index, ulong size);

#endif // SRC_FILES_FILE_