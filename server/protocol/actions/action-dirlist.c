#include "../../ssbs/serializer.h"
#include "../protocol.h"

long
s3_action_dirlist_writer (network_s *network)
{
    s3_response_s *response = network->response;
    response_writer_s writer = response->writer_buffer;
    serializer_t *serialized_data = (serializer_t *) network->request->additional_data;

    if (network->response->header_info.data_length == 0) {
        network->response->header_info.data_length = serialized_data->index;
    }

    response_writer_s response_writer = network->response->writer_buffer;

    long write_available = writer.capacity - writer.index;
    long write_required = response->header_info.data_length - response->total_write_completed;

    long write_todo = write_required > write_available ? write_available : write_required;

    if (write_todo > 0) {
        memcpy (response_writer.address + response_writer.index, serialized_data->memory, write_todo);
        response->total_write_completed += write_todo;
        response->writer_buffer.index += write_todo;
    } else {
        serializer_free (*(serializer_t *) network->request->additional_data);
    }

    m_free(network->request->additional_data);
    network->request->additional_data = NULL;
    return write_todo;
}

int
s3_preprocess_dirlist (s3_request_s *request)
{
    size_t length = 0;
    char *folder_name = s3_attrib_get (request->header_table, S3_ATTRIBNAME_DIRNAME, &length, NULL);
    string_s folder_string = STRING2 (folder_name, length);

    if (filemgmt_folder_exists (folder_string) == FALSE) {
        return S3_RESPONSE_FOLDER_NOT_FOUND;
    }

    if (request->header_info.data_length != 0) {
        return S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT;
    }

    my_list_s *files = filemgmt_folder_files (folder_string);

    serializer_t serializer = serializer_init (256);

    for (size_t i = 0; i < files->count; i++) {
        filemgmt_record_s *record = (filemgmt_record_s *) my_list_get (*files, i);
        sqlite_result_s *results = (sqlite_result_s *) record;

        for (size_t j = 0; j < FILEMGMT_COLUMN_COUNT; j++) {
            s3_dirlist_serialize_sqltype (filemgmt_record_types[j], results + j, &serializer);

            if (filemgmt_record_types[j] == SQLTYPE_TEXT || filemgmt_record_types[j] == SQLTYPE_BLOB) {
                m_free ((results + j)->value._void_p);
            }
        }
    }

    my_list_free (*files);
    m_free (files);

    request->additional_data = m_malloc (sizeof (serializer));
    memcpy (request->additional_data, &serializer, sizeof (serializer));

    return SUCCESS;
}

void
s3_dirlist_serialize_sqltype (enum sqltypes type, sqlite_result_s *result, serializer_t *serializer)
{
    switch (type) {
    case SQLTYPE_BLOB: {
        serializer_add_blob (serializer, NULL, 0, result->value._char_p, result->size);
    } break;
    case SQLTYPE_TEXT: {
        serializer_add_blob (serializer, NULL, 0, result->value._char_p, result->size + 1); // For \0 terminator
    } break;
    case SQLTYPE_LONG: {
        serializer_add_long (serializer, NULL, 0, result->value._int64);
    } break;
    }
}