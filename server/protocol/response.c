#include "protocol.h"

int
dbp_response_writer_update (dbp_response_s *response, string_s write)
{
    string_s writer = response->writer;

    response->header_info.data_length = write.length;

    long avail_write = writer.max_length - writer.length;
    long required_write = write.length - response->data_written;
    long actual_write = required_write > avail_write ? avail_write : required_write;

    if (actual_write > 0)
    {
        memcpy (
          writer.address + writer.length, write.address + response->data_written, actual_write);
        response->data_written += actual_write;
        response->writer.length += actual_write;
    }

    return (actual_write);
}

int
dbp_response_write (dbp_response_s *response, long (*writer) (dbp_response_s *in))
{
    network_s *connection = &((dbp_protocol_s *) response->instance)->connection;

    char *memory = m_malloc (NETWORK_WRITE_BUFFER, MEMORY_FILE_LINE);
    response->writer.address = memory;
    response->writer.length = DBP_PROTOCOL_MAGIC_LEN;
    response->writer.max_length = NETWORK_WRITE_BUFFER;
    response->data_written = 0;

    key_value_pair_s pair = {0};
    pair.key = DBP_RESPONSE_KEY_NAME;
    pair.value = (char *) response->response_code;
    pair.key_length = sizeof (DBP_RESPONSE_KEY_NAME) - 1;
    pair.value_length = 0;

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
    // (for e.g. if the last 4 bits are '0001', adding 15 means '10000'
    // , and last 4 bits are ZEROED. If the last 4 bits are '1111', adding
    // 15 means '11110', and last 4 bits are ZEROED again)
    // and then we ZERO the last 4 of the bits.
    long byte_align = result & 0XF ? (result + 0XF) & ~0XF : result;
    memset (response->writer.address + response->writer.length + result, 0, byte_align - result);

    result = byte_align;
    response->writer.length += result;

    response->header_info.magic = DBP_PROTOCOL_MAGIC;
    response->header_info.header_length = result;
    response->header_info.data_length = 0;
    /*
     * Here it is writer's responsiblity to set the
     * "response->header_info.data_length" upon first call to this function
     */
    long data_written = 0;
    boolean header_written = FALSE;
    while ((data_written = writer (response)), TRUE)
    {
        if (data_written < 0)
        {
            return (DBP_RESPONSE_ERROR_WRITE);
        }

        if (header_written == FALSE)
        {
            *(ulong *) (response->writer.address) = dbp_response_make_magic (response);
        }

        header_written = TRUE;
        network_write_stream (connection, response->writer.address, response->writer.length);

        if (response->data_written == response->header_info.data_length)
        {
            break;
        }
        response->writer.length = 0;
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
    if (buffer)
    {
        for (size_t i = 0; i < response->header_list.count; i++)
        {
            key_value_pair_s pair = *(key_value_pair_s *) my_list_get (response->header_list, i);

            int written = 0;
            if (pair.value_length)
            {
                written = snprintf (
                  buffer + index,
                  buffer_length - index,
                  DBP_RESPONSE_FORMAT_STRING,
                  pair.key_length,
                  pair.key,
                  pair.value_length,
                  pair.value);
            }
            else
            {
                written = snprintf (
                  buffer + index,
                  buffer_length - index,
                  DBP_RESPONSE_FORMAT_LONG,
                  pair.key_length,
                  pair.key,
                  (long) pair.value);
            }
            index += written;

            if (index > DBP_PROTOCOL_HEADER_MAXLEN || index > buffer_length)
            {
                return (-1);
            }
        }
    }
    return (index);
}

ulong
dbp_response_make_magic (dbp_response_s *response)
{
    // first we shift the magic header by 7 bytes
    ulong i = ((ulong) DBP_PROTOCOL_MAGIC << (7 * 8));

    // second then we divide header_length by 16 and right shift by 6 bytes
    i |= ((ulong) response->header_info.header_length >> 4) << (6 * 8);

    i |= (response->header_info.data_length & 0x0000FFFFFFFFFF);
    return (i);
}

int
dbp_response_accept_status (dbp_response_s *response)
{
    network_s *connection = &((dbp_protocol_s *) response->instance)->connection;

    if (connection)
    {
        network_data_atom_s data_read = network_read_long (connection);

        if (data_read.u.long_t == 0XD0FFFFFFFFFFFFFF)
        {
            return (SUCCESS);
        }
    }
    return (FAILED);
}