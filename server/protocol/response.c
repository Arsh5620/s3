#include "protocol.h"
#include "../ssbs/serializer.h"

int
s3_response_writer_update (s3_response_s *response, string_s write)
{
    string_s writer = response->writer_buffer;
    response->header_info.data_length = write.length;

    long avail_write = writer.max_length - writer.length;
    long required_write = write.length - response->total_write_completed;
    long actual_write = required_write > avail_write ? avail_write : required_write;

    if (actual_write > 0)
    {
        memcpy (
          writer.address + writer.length,
          write.address + response->total_write_completed,
          actual_write);
        response->total_write_completed += actual_write;
        response->writer_buffer.length += actual_write;
    }

    return (actual_write);
}

int
s3_response_write (s3_response_s *response, long (*writer) (s3_response_s *))
{
    network_s *connection = &((s3_protocol_s *) response->instance)->connection;

    char *memory = m_calloc (NETWORK_WRITE_BUFFER);
    response->writer_buffer.address = memory;
    response->writer_buffer.length = S3_PROTOCOL_MAGIC_LEN;
    response->writer_buffer.max_length = NETWORK_WRITE_BUFFER;
    response->total_write_completed = 0;

    key_value_pair_s pair = {0};
    pair.key = S3_RESPONSE_KEY_NAME;
    pair.value = 0;
    pair.key_length = sizeof (S3_RESPONSE_KEY_NAME) - 1;
    pair.value_length = response->response_code;

    if (response->header_list.address == NULL)
    {
        response->header_list = my_list_new (1, sizeof (key_value_pair_s));
    }
    my_list_push (&response->header_list, (char *) &pair);

    long result = s3_response_write_header (
      response, memory + S3_PROTOCOL_MAGIC_LEN, NETWORK_WRITE_BUFFER - S3_PROTOCOL_MAGIC_LEN);

    if (result == -1)
    {
        return (S3_RESPONSE_ERROR_WRITING_HEADERS);
    }

    // align the header on a 16 byte boundary
    // we are checking if the result is on a 16 byte boundary,
    // if it is not, then we add 15 which increments the boundary
    // and then we ZERO the last 4 of the bits.
    result = result & 0XF ? (result + 0XF) & ~0XF : result;

    response->writer_buffer.length += result;

    response->header_info.magic = S3_PROTOCOL_MAGIC;
    response->header_info.header_length = result;
    response->header_info.data_length = 0;
    /*
     * Here it is writer's responsiblity to set the
     * "response->header_info.data_length" upon first call to this function
     */
    long total_write_completed = 0;
    int error = NETWORK_SUCCESS; // NETWORK_SUCCESS is alias for SUCCESS
    boolean header_written = FALSE;
    while (TRUE)
    {
        total_write_completed = writer (response);

        if (total_write_completed < 0)
        {
            return (S3_RESPONSE_NETWORK_ERROR_WRITE);
        }

        if (header_written == FALSE)
        {
            *(ulong *) (response->writer_buffer.address) = s3_response_make_magic (response);
        }

        header_written = TRUE;
        int error = network_write_stream (
          connection, response->writer_buffer.address, response->writer_buffer.length);

        if (error != NETWORK_SUCCESS)
        {
            break;
        }

        if (response->total_write_completed == response->header_info.data_length)
        {
            break;
        }
        response->writer_buffer.length = 0;
    }

    m_free (memory);
    my_list_free (response->header_list);
    response->header_list = (my_list_s){0};

    return (error);
}

long
s3_response_write_header (s3_response_s *response, char *buffer, ulong buffer_length)
{
    long index = 0;
    if (buffer != NULL)
    {
        serializer_t serializer = serializer_init ();

        for (size_t i = 0; i < response->header_list.count; i++)
        {
            key_value_pair_s pair = *(key_value_pair_s *) my_list_get (response->header_list, i);

            if (pair.value != NULL)
            {
                serializer_add_blob (
                  &serializer, pair.key, pair.key_length, pair.value, pair.value_length);
            }
            else
            {
                serializer_add_long (&serializer, pair.key, pair.key_length, pair.value_length);
            }
        }

        if (serializer.index > S3_PROTOCOL_HEADER_MAXLEN || serializer.index > buffer_length)
        {
            serializer_free (serializer);
            return (-1);
        }
        else
        {
            memcpy (buffer, serializer.memory, serializer.index);
            serializer_free (serializer);
        }

        index = serializer.index;
    }

    return (index);
}

ulong
s3_response_make_magic (s3_response_s *response)
{
    // first we shift the magic header by 7 bytes
    ulong magic = ((ulong) S3_PROTOCOL_MAGIC << (7 * 8));

    // second then we divide header_length by 16 and right shift by 6 bytes
    magic |= ((ulong) response->header_info.header_length >> 4) << (6 * 8);

    magic |= (response->header_info.data_length & 0x0000FFFFFFFFFF);
    return (magic);
}

int
s3_response_accept_status (s3_response_s *response)
{
    s3_protocol_s *protocol = ((s3_protocol_s *) response->instance);
    network_s *connection = &protocol->connection;

    if (connection)
    {
        long magic;
        int error = s3_network_read_stream_async (protocol, (char *) &magic, sizeof (long));
        if (error != S3_RESPONSE_SUCCESS || protocol->current.read_status == STATUS_ASYNC_COMPLETED)
        {
            return error;
        }

        if (magic != 0x0000545045434341) // ACCEPT\0\0 in string
        {
            return (S3_RESPONSE_INVALID_COMMUNICATION);
        }
    }
    return S3_RESPONSE_SUCCESS;
}