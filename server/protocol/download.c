#include "protocol.h"
#include <openssl/sha.h>

int
dbp_request_data (dbp_protocol_s *protocol, dbp_request_s *request)
{
    if (dbp_request_data_headers (protocol, request) == SUCCESS)
    {
        if (dbp_file_download (request) == SUCCESS)
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
dbp_request_data_headers (dbp_protocol_s *protocol, dbp_request_s *request)
{
    int error;
    int magic = network_read_primitives (&protocol->connection, sizeof (int32_t), &error);

    if (magic != 0X444E4553) // SEND in ascii
    {
        return (FAILED);
    }

    size_t data_length = network_read_primitives(&protocol->connection, sizeof(size_t), & error);

    if (request->header_info.data_length != data_length)
    {
        return (FAILED);
    }
    return (SUCCESS);
}

void
dbp_file_hash_sha1 (file_sha1 *hash, network_data_s data, boolean fin)
{
    if (hash->hash_buffer == NULL)
    {
        hash->hash_buffer = m_calloc (FILE_BUFFER_LENGTH);
        hash->hash_size = (FILE_BUFFER_LENGTH);
        hash->hash_index = 0;
        // default element count is 10, and each element is 20 in length
        hash->hash_list = my_list_new (10, 20);
    }

    ulong write = (hash->hash_size - hash->hash_index);
    ulong actual_write = data.data_length;

    if (write > actual_write)
    {
        write = actual_write;
    }

    // first lets copy the data required.
    if (write)
    {
        memcpy (hash->hash_buffer + hash->hash_index, data.data_address, write);
    }

    hash->hash_index += write;
    if (hash->hash_index == hash->hash_size || (fin && hash->hash_index))
    {
        char sha1[20];
        unsigned char *digest
          = SHA1 ((unsigned char *) hash->hash_buffer, hash->hash_size, (unsigned char *) sha1);

        my_list_push (&hash->hash_list, (char *) digest);

        ulong diff = actual_write - write;
        memcpy (hash->hash_buffer, data.data_address, diff);
        memset (hash->hash_buffer + diff, NULL_ZERO, hash->hash_size - diff);
        hash->hash_index = diff;
    }
}

int
dbp_file_hash_writesha1 (char *file_name, my_list_s list)
{
    FILE *file = fopen (file_name, FILE_MODE_WRITEBINARY);
    char *sha1_hex = m_malloc (41);
    for (ulong i = 0; i < list.count; ++i)
    {
        // considering our list size is 20 bytes constant.
        // to translate this into hex values, it will be 40 bytes

        unsigned char *digest = (unsigned char *) my_list_get (list, i);
        for (size_t j = 0; j < 20; j++)
        {
            snprintf (sha1_hex + (j * 2), 3, "%02X", *(digest + j));
        }
        *(sha1_hex + 40) = '\n';

        if (fwrite (sha1_hex, 1, 41, file) != 41)
        {
            return (FAILED);
        }
    }
    fflush (file);
    return (SUCCESS);
}

int
dbp_file_download (dbp_request_s *request)
{
    char *temp_file;
    file_info_s fileinfo = {0};
    file_sha1 hash = {0};
    fileinfo.size = request->header_info.data_length;
    temp_file = request->file_name.temp_file_name.address;

    FILE *temp = fopen (temp_file, FILE_MODE_WRITEBINARY);

    if (temp == NULL)
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, PROTOCOL_DOWNLOAD_FILE_NOOPEN);
        return (FAILED);
    }

    clock_t starttime = clock ();

    dbp_protocol_s *protocol = (dbp_protocol_s *) request->instance;
    int download_status
      = file_download (temp, &protocol->connection, fileinfo.size, &hash, dbp_file_hash_sha1);

    dbp_file_hash_writesha1 (request->file_name.temp_hash_file_name.address, hash.hash_list);

    m_free (hash.hash_buffer);

    clock_t endtime = clock ();

    double time_elapsed = (((double) (endtime - starttime)) / CLOCKS_PER_SEC) * 1000;

    // file download size in bits/second
    double speed = (((double) fileinfo.size / 1024 / 128) * (1000 / time_elapsed));

    my_print (
      MESSAGE_OUT_LOGS,
      LOGGER_LEVEL_INFO,
      PROTOCOL_DOWNLOAD_COMPLETE,
      request->file_name.file_name.length,
      request->file_name.file_name.address,
      fileinfo.size,
      download_status,
      time_elapsed,
      speed);

    fclose (temp);
    return (SUCCESS);
}