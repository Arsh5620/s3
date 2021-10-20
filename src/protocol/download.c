#include "protocol.h"

#include <openssl/sha.h>

void
s3_file_hash (file_hash_s *hash, network_read_s data, boolean fin)
{
    if (hash->hash_buffer == NULL) {
        hash->hash_buffer = m_calloc (FILE_BUFFER_LENGTH);
        hash->hash_size = FILE_BUFFER_LENGTH;
        hash->hash_index = 0;
        hash->hash_list = my_list_new (10, 20);
    }

    ulong write = hash->hash_size - hash->hash_index;
    ulong actual_write = data.length;

    if (write > actual_write) {
        write = actual_write;
    }

    if (write) {
        memcpy (hash->hash_buffer + hash->hash_index, data.address, write);
    }

    hash->hash_index += write;

    if (hash->hash_index == hash->hash_size || (fin && hash->hash_index)) {
        char hash_buffer[hash->hash_compute_length];

        hash->hash_function (hash, hash_buffer, hash->hash_compute_length);

        /**
         * move is required if we still have more data remaining
         * after our buffer was filled and we have already generated
         * a hash
         */
        ulong move_required = actual_write - write;
        memcpy (hash->hash_buffer, data.address, move_required);
        hash->hash_index = move_required;
    }
}

void
s3_file_hash_sha1 (void *file_hash_address, char *hash_buffer, int hash_buffer_length)
{
    file_hash_s *hash = (file_hash_s *) file_hash_address;

    unsigned char *digest = SHA1 ((unsigned char *) hash->hash_buffer, hash->hash_size, (unsigned char *) hash_buffer);

    my_list_push (&hash->hash_list, (char *) digest);
}

int
s3_file_hash_write (char *file_name, file_hash_s hash)
{
    FILE *file = fopen (file_name, FILE_MODE_WRITEBINARY);
    int allocation_length = hash.hash_compute_length * 2 + 1;
    char *hash_string = m_malloc (allocation_length);
    *(hash_string + (hash.hash_compute_length * 2)) = '\n';

    my_list_s hash_list = hash.hash_list;
    for (ulong i = 0; i < hash_list.count; ++i) {
        unsigned char *digest = (unsigned char *) my_list_get (hash_list, i);
        string_hexify ((uchar *) hash_string, digest, hash.hash_compute_length);

        if (fwrite (hash_string, sizeof (char), allocation_length, file) != allocation_length) {
            return (FAILED);
        }
    }

    fflush (file);
    return (SUCCESS);
}