#include "../protocol.h"

long
s3_action_request_writer (network_s *network)
{
    s3_response_s *response = network->response;
    response_writer_s writer = response->writer_buffer;
    FILE *file = fopen (response->file_name->real_file_name.address, FILE_MODE_READBINARY);

    if (response->header_info.data_length == 0) {
        response->header_info.data_length = file_stat (file).st_size;
    }

    fseek (file, response->total_write_completed, SEEK_SET);

    long write_available = writer.capacity - writer.index;
    long write_required = response->header_info.data_length - response->total_write_completed;

    long write_todo = write_required > write_available ? write_available : write_required;

    if (write_todo > 0) {
        fread (writer.address + writer.index, sizeof (char), write_todo, file);
        response->total_write_completed += write_todo;
        response->writer_buffer.index += write_todo;
    }

    fclose (file);
    return write_todo;
}

int
s3_preprocess_request (s3_request_s *request)
{
    if (filemgmt_file_exists (request->file_name.file_name, request->file_name.real_file_name, NULL) == FALSE) {
        return (S3_RESPONSE_FILE_NOT_FOUND);
    }

    if (request->header_info.data_length != 0) {
        return (S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT);
    }

    request->data_write_confirm = TRUE;
    return (SUCCESS);
}
