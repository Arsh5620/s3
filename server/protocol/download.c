#include "protocol.h"
#include "../crypto/sha.h"
#include <openssl/sha.h>

int
s3_request_data (s3_protocol_s *protocol, s3_request_s *request)
{
    if (s3_request_data_headers (protocol, request) == SUCCESS)
    {
        if (s3_file_download (request) == SUCCESS)
        {
            return (DBP_RESPONSE_SUCCESS);
        }
        else
        {
            return DBP_RESPONSE_SERVER_INTERNAL_ERROR;
        }
    }
    else
    {
        return (DBP_RESPONSE_CORRUPTED_DATA_HEADERS);
    }
}

/*
 * headers for data has 2 byte magic, 0xd0,0xd1 and 6 byte length
 */
int
s3_request_data_headers (s3_protocol_s *protocol, s3_request_s *request)
{
    int error;
    int magic = network_read_primitives (&protocol->connection, sizeof (int32_t), &error);

    if (magic != 0X444E4553) // SEND in ascii
    {
        return (FAILED);
    }

    size_t data_length = network_read_primitives (&protocol->connection, sizeof (size_t), &error);

    if (request->header_info.data_length != data_length)
    {
        return (FAILED);
    }
    return (SUCCESS);
}

void
s3_file_hash (file_hash_s *hash, network_data_s data, boolean fin)
{
    if (hash->hash_buffer == NULL)
    {
        hash->hash_buffer = m_calloc (FILE_BUFFER_LENGTH);
        hash->hash_size = FILE_BUFFER_LENGTH;
        hash->hash_index = 0;
        hash->hash_list = my_list_new (10, 20);
    }

    ulong write = hash->hash_size - hash->hash_index;
    ulong actual_write = data.data_length;

    if (write > actual_write)
    {
        write = actual_write;
    }

    if (write)
    {
        memcpy (hash->hash_buffer + hash->hash_index, data.data_address, write);
    }

    hash->hash_index += write;

    if (hash->hash_index == hash->hash_size || (fin && hash->hash_index))
    {
        char hash_buffer[hash->hash_compute_length];

        hash->hash_function (hash, hash_buffer, hash->hash_compute_length);

        /**
         * move is required if we still have more data remaining
         * after our buffer was filled and we have already generated
         * a hash
         */
        ulong move_required = actual_write - write;
        memcpy (hash->hash_buffer, data.data_address, move_required);
        hash->hash_index = move_required;
    }
}

void
s3_file_hash_sha1 (void *file_hash_address, char *hash_buffer, int hash_buffer_length)
{
    file_hash_s *hash = (file_hash_s *) file_hash_address;

    unsigned char *digest
      = SHA1 ((unsigned char *) hash->hash_buffer, hash->hash_size, (unsigned char *) hash_buffer);

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
    for (ulong i = 0; i < hash_list.count; ++i)
    {
        unsigned char *digest = (unsigned char *) my_list_get (hash_list, i);
        for (size_t j = 0; j < hash.hash_compute_length; j++)
        {
            hash_string[j * 2 + 1] = sha_tohex (digest[j] & 0xF);
            hash_string[j * 2] = sha_tohex ((digest[j] >> 4) & 0xF);
        }

        if (fwrite (hash_string, sizeof (char), allocation_length, file) != allocation_length)
        {
            return (FAILED);
        }
    }

    fflush (file);
    return (SUCCESS);
}

int
s3_file_download (s3_request_s *request)
{
    file_info_s fileinfo = {0};
    file_hash_s hash = {0};
    hash.hash_compute_length = 20; // Length of SHA1 hash
    hash.hash_function = s3_file_hash_sha1;

    fileinfo.size = request->header_info.data_length;
    char *temp_file = request->file_name.temp_file_name.address;

    FILE *temp = fopen (temp_file, FILE_MODE_WRITEBINARY);

    if (temp == NULL)
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, PROTOCOL_DOWNLOAD_FILE_NOOPEN);
        return (FAILED);
    }

    clock_t start_time = clock ();

    s3_protocol_s *protocol = (s3_protocol_s *) request->instance;
    int download_status
      = file_download (temp, &protocol->connection, fileinfo.size, &hash, s3_file_hash);

    s3_file_hash_write (request->file_name.temp_hash_file_name.address, hash);

    m_free (hash.hash_buffer);

    clock_t endtime = clock ();

    long time_elapsed = (endtime - start_time) / (CLOCKS_PER_SEC / 1000);

    my_print (
      MESSAGE_OUT_LOGS,
      LOGGER_LEVEL_INFO,
      PROTOCOL_DOWNLOAD_COMPLETE,
      request->file_name.file_name.length,
      request->file_name.file_name.address,
      fileinfo.size,
      download_status,
      time_elapsed);

    fclose (temp);
    return (SUCCESS);
}