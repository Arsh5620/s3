#include "../protocol.h"

long
dbp_action_request_writer (dbp_response_s *response)
{
    string_s writer = response->writer_buffer;
    FILE *file = fopen (response->file_name->real_hash_file_name.address, FILE_MODE_READBINARY);

    if (response->header_info.data_length == 0)
    {
        response->header_info.data_length = file_stat (file).st_size;
    }

    fseek (file, response->total_write_completed, SEEK_SET);

    long write_available = writer.max_length - writer.length;
    long write_required = response->header_info.data_length - response->total_write_completed;

    long write_todo = write_required > write_available ? write_available : write_required;

    if (write_todo > 0)
    {
        fread (writer.address + writer.length, sizeof (char), write_todo, file);
        response->total_write_completed += write_todo;
        response->writer_buffer.length += write_todo;
    }

    return (write_todo);
}

int
dbp_prehook_request (dbp_request_s *request)
{
    if (!filemgmt_file_exists (
          request->file_name.file_name, request->file_name.real_file_name, NULL))
    {
        return (DBP_RESPONSE_FILE_NOT_FOUND);
    }

    if (request->header_info.data_length != 0)
    {
        return (DBP_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT);
    }

    if (access (request->file_name.real_hash_file_name.address, F_OK) != SUCCESS)
    {
        return (DBP_RESPONSE_FILE_NOT_FOUND);
    }

    request->data_write_confirm = TRUE;
    return (SUCCESS);
}
