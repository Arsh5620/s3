#include "protocol.h"
#include "../ssbs/serializer.h"

int
dbp_response_writer_update (dbp_response_s *response, string_s write)
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
dbp_response_write (dbp_response_s *response, long (*writer) (dbp_response_s *))
{
    network_s *connection = &((dbp_protocol_s *) response->instance)->connection;

    char *memory = m_calloc (NETWORK_WRITE_BUFFER, MEMORY_FILE_LINE);
    response->writer_buffer.address = memory;
    response->writer_buffer.length = DBP_PROTOCOL_MAGIC_LEN;
    response->writer_buffer.max_length = NETWORK_WRITE_BUFFER;
    response->total_write_completed = 0;

    key_value_pair_s pair = {0};
    pair.key = DBP_RESPONSE_KEY_NAME;
    pair.value = 0;
    pair.key_length = sizeof (DBP_RESPONSE_KEY_NAME) - 1;
    pair.value_length = response->response_code;

    if (response->header_list.address == NULL)
    {
        response->header_list = my_list_new (1, sizeof (key_value_pair_s));
    }
    my_list_push (&response->header_list, (char *) &pair);

    long result = dbp_response_write_header (
      response, memory + DBP_PROTOCOL_MAGIC_LEN, NETWORK_WRITE_BUFFER - DBP_PROTOCOL_MAGIC_LEN);

    if (result == -1)
    {
        return (DBP_RESPONSE_ERROR_WRITING_HEADERS);
    }

    // align the header on a 16 byte boundary
    // we are checking if the result is on a 16 byte boundary,
    // if it is not, then we add 15 which increments the boundary
    // and then we ZERO the last 4 of the bits.
    result = result & 0XF ? (result + 0XF) & ~0XF : result;

    response->writer_buffer.length += result;

    response->header_info.magic = DBP_PROTOCOL_MAGIC;
    response->header_info.header_length = result;
    response->header_info.data_length = 0;
    /*
     * Here it is writer's responsiblity to set the
     * "response->header_info.data_length" upon first call to this function
     */
    long total_write_completed = 0;
    boolean header_written = FALSE;
    while (TRUE)
    {
        total_write_completed = writer (response);

        if (total_write_completed < 0)
        {
            return (DBP_RESPONSE_NETWORK_ERROR_WRITE);
        }

        if (header_written == FALSE)
        {
            *(ulong *) (response->writer_buffer.address) = dbp_response_make_magic (response);
        }

        header_written = TRUE;
        network_write_stream (
          connection, response->writer_buffer.address, response->writer_buffer.length);

        if (response->total_write_completed == response->header_info.data_length)
        {
            break;
        }
        response->writer_buffer.length = 0;
    }

    m_free (memory, MEMORY_FILE_LINE);
    my_list_free (response->header_list);
    response->header_list = (my_list_s){0};

    return (SUCCESS);
}

long
dbp_response_write_header (dbp_response_s *response, char *buffer, ulong buffer_length)
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

        if (serializer.index > DBP_PROTOCOL_HEADER_MAXLEN || serializer.index > buffer_length)
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
dbp_response_make_magic (dbp_response_s *response)
{
    // first we shift the magic header by 7 bytes
    ulong magic = ((ulong) DBP_PROTOCOL_MAGIC << (7 * 8));

    // second then we divide header_length by 16 and right shift by 6 bytes
    magic |= ((ulong) response->header_info.header_length >> 4) << (6 * 8);

    magic |= (response->header_info.data_length & 0x0000FFFFFFFFFF);
    return (magic);
}

int
dbp_response_accept_status (dbp_response_s *response)
{
    network_s *connection = &((dbp_protocol_s *) response->instance)->connection;

    if (connection)
    {
        long long data_read = network_read_primitives (connection, sizeof (long long), NULL);

        if (data_read == 0x0000545045434341) // ACCEPT\0\0 in string
        {
            return (SUCCESS);
        }
    }
    return (FAILED);
}