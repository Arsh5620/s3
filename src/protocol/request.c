#include "../ssbs/deserializer.h"
#include "protocol.h"
#include <string.h>

int
s3_network_read_stream_async (s3_protocol_s *protocol, char *memory, int size)
{
    // network_init_async_read (&protocol->current.read_info, size);
    // network_data_s data = network_read_stream (&protocol->connection, &protocol->current.read_info);

    // if (data.error_code == NETWORK_ASYNC_WOULDBLOCK)
    // {
    //     protocol->current.read_status = STATUS_ASYNC_MORE;
    //     return SUCCESS;
    // }
    // else if (data.error_code != SUCCESS)
    // {
    //     logs_put (MESSAGE_OUT_LOGS, LOGGER_LEVEL_ERROR, NETWORK_READ_ERROR);
    //     return S3_RESPONSE_NETWORK_ERROR_READ;
    // }

    // if (memory != NULL)
    // {
    //     memcpy ((char *) memory, protocol->current.read_info.current_read_memory, size);
    //     s3_network_data_free (protocol);
    //     network_complete_async_read (&protocol->current.read_info);
    // }
    // protocol->current.read_status = STATUS_ASYNC_COMPLETED;
    return SUCCESS;
}

void
s3_network_data_free (s3_protocol_s *protocol)
{
    // if (protocol->current.read_info.current_read_memory)
    // {
    //     m_free (protocol->current.read_info.current_read_memory);
    //     protocol->current.read_info.current_read_memory = NULL;
    // }
}

s3_header_s
s3_request_read_magic (size_t magic, boolean *valid)
{
    s3_header_s header_info = s3_header_parse8 (magic);

    if (header_info.magic != S3_PROTOCOL_MAGIC) {
        logs_put (LOG_OUT_LOGS, LOG_LEVEL_ERROR, PROTOCOL_ABORTED_CORRUPTION, magic);
        *valid = FALSE;
    } else {
        *valid = TRUE;
    }

    return header_info;
}

// int
// s3_request_read_headers (s3_protocol_s *protocol, s3_request_s *request)
// {
//     /**
//      * headers is needed even after we have deserialized the binary information
//      * because the deserializer will only create a struct with pointers to the original data
//      */
// }

void
s3_print_headers (my_list_s list)
{
    logs_put (LOG_OUT_LOGS, LOG_LEVEL_DEBUG, DESERIALIZER_PRINT_HEADERS, list.count);

    for (size_t i = 0; i < list.count; i++) {
        deserializer_value_t *pair = (deserializer_value_t *) my_list_get (list, i);

        logs_put (LOG_OUT_LOGS,
                  LOG_LEVEL_DEBUG,
                  DESERIALIZER_PRINT_HEADERS_ROW_STRING,
                  pair->key_length,
                  pair->key,
                  pair->value_long,
                  pair->value_pointer,
                  pair->value_long,
                  pair->value_float,
                  pair->value_double);
    }
}

deserializer_value_t *
list_find_key_string (my_list_s list, char *key, int key_length)
{
    for (size_t i = 0; i < list.count; i++) {
        deserializer_value_t *pair = (deserializer_value_t *) my_list_get (list, 0);

        if (pair != NULL && pair->key_length == key_length && memcmp (pair->key, key, key_length) == 0) {
            return pair;
        }
    }
    return NULL;
}

int
list_index_key_string (my_list_s list, char *key, int key_length)
{
    for (size_t i = 0; i < list.count; i++) {
        deserializer_value_t *pair = (deserializer_value_t *) my_list_get (list, 0);

        if (pair != NULL && pair->key_length == key_length && memcmp (pair->key, key, key_length) == 0) {
            return i;
        }
    }
    return -1;
}

int
s3_request_read_action (s3_request_s *request)
{
    deserializer_value_t *kvpair;
    my_list_s list = request->header_list;

    char *action = attribs[S3_ATTRIB_ACTION];
    int action_len = strlen (action);

    if (list.count > 0) {
        kvpair = list_find_key_string (list, action, action_len);
    } else {
        return S3_RESPONSE_HEADER_EMPTY;
    }

    if (kvpair != NULL) {
        request->action = -1;
        for (int i = 0; i < actions_count; i++) {
            if (strcmp (actions[i], kvpair->value_pointer)) {
                request->action = i;
                break;
            }
        }
        return SUCCESS;
    } else {
        return S3_RESPONSE_ACTION_INVALID;
    }
}

s3_header_s inline s3_header_parse8 (size_t magic)
{
    /* shr by 6 bytes, and multiply by 16 to get header size */
    s3_header_s header = {.header_length = (((magic & 0x00FF000000000000) >> (6 * 8)) * 16),
                          .data_length = (magic & 0x0000FFFFFFFFFFFF),
                          .magic = ((magic & 0xFF00000000000000) >> (7 * 8))};

    return (header);
}