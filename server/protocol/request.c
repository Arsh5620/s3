#include "protocol.h"
#include "../ssbs/deserializer.h"

ulong
s3_request_read_headers (s3_protocol_s protocol, s3_request_s *request)
{
    int error;
    long long magic = network_read_primitives (&protocol.connection, sizeof (long long), &error);

    if (error != SUCCESS)
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, NETWORK_READ_ERROR);
        return (S3_RESPONSE_NETWORK_ERROR_READ);
    }

    request->header_info = s3_header_parse8 (magic);

    if (request->header_info.magic != S3_PROTOCOL_MAGIC)
    {
        my_print (
          MESSAGE_OUT_LOGS,
          LOGGER_LEVEL_ERROR,
          PROTOCOL_ABORTED_CORRUPTION,
          request->header_info.magic);
        return (S3_RESPONSE_CORRUPTED_PACKET);
    }

    /**
     * headers is needed even after we have deserialized the binary information
     * because the deserializer will only create a struct with pointers to the original data
     */
    network_data_s headers
      = network_read_stream (&protocol.connection, request->header_info.header_length);
    request->header_raw = headers;

    if (headers.error_code)
    {
        my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, PROTOCOL_READ_HEADERS_FAILED);
        return (S3_RESPONSE_NETWORK_ERROR_READ);
    }

    request->header_list = s3_deserialize_headers (headers, &error);

    if (error != SUCCESS)
    {
        return error;
    }
    else
    {
        s3_print_headers (request->header_list);
        return (S3_RESPONSE_SUCCESS);
    }
}

void
s3_print_headers (my_list_s list)
{
    my_print (MESSAGE_OUT_LOGS, LOGGER_LEVEL_DEBUG, DESERIALIZER_PRINT_HEADERS, list.count);

    for (size_t i = 0; i < list.count; i++)
    {
        key_value_pair_s *pair = (key_value_pair_s *) my_list_get (list, i);

        my_print (
          MESSAGE_OUT_LOGS,
          LOGGER_LEVEL_DEBUG,
          DESERIALIZER_PRINT_HEADERS_ROW_STRING,
          pair->key_length,
          pair->key,
          pair->value_length,
          pair->value);
    }
}

my_list_s
s3_deserialize_headers (network_data_s headers, int *error)
{
    deserializer_t deserializer = deserializer_init (headers.data_address, headers.data_length);
    my_list_s binary_list = my_list_new (16, sizeof (deserializer_value_t));
    my_list_s kvpairs_list = my_list_new (16, sizeof (key_value_pair_s));

    deserialize_all (&deserializer, &binary_list);
    if (s3_copy_keyvaluepairs (binary_list, &kvpairs_list) != SUCCESS)
    {
        /* this means that an error occured while processing the input */
        if (error != NULL)
        {
            *error = S3_RESPONSE_DESERIALIZER_ERROR;
        }
    }

    my_list_free (binary_list);
    if (error != NULL)
    {
        *error = SUCCESS;
    }

    return kvpairs_list;
}

int
s3_copy_keyvaluepairs (my_list_s source_list, my_list_s *dest_list)
{
    for (int i = 0; i < source_list.count; ++i)
    {
        deserializer_value_t result = *(deserializer_value_t *) my_list_get (source_list, i);
        if (result.key == 0 || result.key_size == 0)
        {
            my_print (
              MESSAGE_OUT_LOGS,
              LOGGER_LEVEL_WARN,
              "Binary deserializer: Ignored empty key, %s:%s",
              result.key,
              result.value_2);
            return FAILED;
        }

        key_value_pair_s pair = {
          .is_valid = TRUE,
          .key = result.key,
          .key_length = result.key_size,
          .value = result.value_2,
          .value_length = result.value_1,
        };

        my_list_push (dest_list, (char *) &pair);
    }
    return SUCCESS;
}

int
s3_request_read_action (s3_request_s *request)
{
    key_value_pair_s pair = {0};
    my_list_s list = request->header_list;
    if (list.count > 0)
    {
        pair = *(key_value_pair_s *) my_list_get (list, 0);
    }
    else
    {
        return (S3_RESPONSE_HEADER_EMPTY);
    }

    data_keys_s action = attribs[0];
    int actionval = S3_ACTION_INVALID;
    if (action.strlen == pair.key_length && memcmp (pair.key, action.string, action.strlen) == 0)
    {
        // now here to check the action that the client is requesting.
        actionval = binary_search (
          actions,
          sizeof (data_keys_s),
          actions_count,
          pair.value,
          pair.value_length,
          data_string_compare);
    }

    if (actionval == S3_ACTION_INVALID)
    {
        return (S3_RESPONSE_ACTION_INVALID);
    }
    request->action = actions[actionval].attrib_code;
    return (S3_RESPONSE_SUCCESS);
}

// returns TRUE on success and FALSE otherwise
int
s3_attrib_contains (hash_table_s table, int attrib)
{
    hash_input_u key = {.number = attrib};
    hash_table_bucket_s bucket = hash_table_get (table, key, NULL_ZERO);
    if (bucket.is_occupied == 0 && attrib != NULL_ZERO)
    {
        return (FALSE);
    }
    return (TRUE);
}

// this function should be called before dispatching the request
// to make sure that the header contains all the required key:value
// pairs needed by the called function.
int
s3_attribs_assert (hash_table_s table, enum s3_attribs_enum *match, int count)
{
    for (long i = 0; i < count; i++)
    {
        enum s3_attribs_enum attrib = match[i];
        if (s3_attrib_contains (table, attrib) == FALSE)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}

s3_header_s inline s3_header_parse8 (size_t magic)
{
    /* shr by 6 bytes, and multiply by 16 to get header size */
    s3_header_s header = {
      .header_length = (((magic & 0x00FF000000000000) >> (6 * 8)) * 16),
      .data_length = (magic & 0x0000FFFFFFFFFFFF),
      .magic = ((magic & 0xFF00000000000000) >> (7 * 8))};

    return (header);
}