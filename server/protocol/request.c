#include "protocol.h"
#include "../ssbs/deserializer.h"

ulong
dbp_request_read_headers (dbp_protocol_s protocol, dbp_request_s *request)
{
    network_data_atom_s header_read = network_read_long (&protocol.connection);
    request->header_info = dbp_header_parse8 (header_read.u.long_t);

    if (request->header_info.magic != DBP_PROTOCOL_MAGIC)
    {
        output_handle (
          OUTPUT_HANDLE_LOGS,
          LOGGER_LEVEL_ERROR,
          PROTOCOL_ABORTED_CORRUPTION,
          request->header_info.magic);
        return (DBP_RESPONSE_CORRUPTED_PACKET);
    }

    network_data_s header_raw
      = network_read_stream (&protocol.connection, request->header_info.header_length);

    request->header_raw = header_raw;
    if (header_raw.error_code)
    {
        output_handle (OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR, PROTOCOL_READ_HEADERS_FAILED);
        return (DBP_RESPONSE_ERROR_READ);
    }

    deserializer_t deserializer
      = deserializer_init (header_raw.data_address, header_raw.data_length);
    my_list_s header_list_src = my_list_new (16, sizeof (deserializer_value_t));
    my_list_s header_list_binary = my_list_new (16, sizeof (key_value_pair_s));

    deserialize_all (&deserializer, &header_list_src);
    if (dbp_copy_keyvaluepairs (header_list_src, &header_list_binary) != SUCCESS)
    {
        /* this means that an error occured while processing the input */
        return (DBP_RESPONSE_PARSE_ERROR);
    }

    my_list_free (header_list_src);
    request->header_list = header_list_binary;
    return (DBP_RESPONSE_SUCCESS);
}

int
dbp_copy_keyvaluepairs (my_list_s source_list, my_list_s *dest_list)
{
    for (int i = 0; i < source_list.count; ++i)
    {
        deserializer_value_t result = *(deserializer_value_t *) my_list_get (source_list, i);
        if (result.key == 0 || result.key_size == 0)
        {
            output_handle (
              OUTPUT_HANDLE_LOGS,
              LOGGER_LEVEL_WARN,
              "Binary deserializer: Ignored empty key, %s:%s", result.key, result.value_2);
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
dbp_request_read_action (dbp_request_s *request)
{
    key_value_pair_s pair = {0};
    my_list_s list = request->header_list;
    if (list.count > 0)
    {
        pair = *(key_value_pair_s *) my_list_get (list, 0);
    }
    else
    {
        return (DBP_RESPONSE_HEADER_EMPTY);
    }

    data_keys_s action = attribs[0];
    int actionval = DBP_ACTION_NOTVALID;
    if (action.strlen == pair.key_length && memcmp (pair.key, action.string, action.strlen) == 0)
    {
        // now here to check the action that the client is requesting.
        actionval = binary_search (
          actions,
          sizeof (data_keys_s),
          DBP_ACTIONS_COUNT,
          pair.value,
          pair.value_length,
          data_key_compare);
    }

    if (actionval == DBP_ACTION_NOTVALID)
    {
        return (DBP_RESPONSE_ACTION_INVALID);
    }
    request->action = actions[actionval].attrib_code;
    return (DBP_RESPONSE_SUCCESS);
}

// returns TRUE on success and FALSE otherwise
int
dbp_attrib_contains (hash_table_s table, int attrib)
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
dbp_attribs_assert (hash_table_s table, enum dbp_attribs_enum *match, int count)
{
    for (long i = 0; i < count; i++)
    {
        enum dbp_attribs_enum attrib = match[i];
        if (dbp_attrib_contains (table, attrib) == FALSE)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}

dbp_header_s inline dbp_header_parse8 (size_t magic)
{
    /* shr by 6 bytes, and multiply by 16 to get header size */
    dbp_header_s header = {
      .header_length = (((magic & 0x00FF000000000000) >> (6 * 8)) * 16),
      .data_length = (magic & 0x0000FFFFFFFFFFFF),
      .magic = ((magic & 0xFF00000000000000) >> (7 * 8))};

    return (header);
}