#include "../protocol.h"

long
dbp_action_request_writer (dbp_response_s *in)
{
    string_s writer = in->writer;
    FILE *fw = fopen (in->file_info->real_hash_file_name.address, FILE_MODE_READBINARY);
    if (in->header_info.data_length == 0)
    {
        struct stat fs = file_stat (fw);
        in->header_info.data_length = fs.st_size;
    }
    fseek (fw, in->data_written, SEEK_SET);

    long avail_write = writer.max_length - writer.length;
    long required_write = in->header_info.data_length - in->data_written;
    long to_write = required_write > avail_write ? avail_write : required_write;

    if (to_write > 0)
    {
        fread (writer.address + writer.length, 1, to_write, fw);
        in->data_written += to_write;
        in->writer.length += to_write;
    }
    return (to_write);
}

int
dbp_posthook_request (dbp_request_s *request, dbp_response_s *response)
{
    if (access (request->file_info.real_hash_file_name.address, F_OK) == -1)
    {
        return (DBP_RESPONSE_FILE_NOT_FOUND);
    }

    response->response_code = DBP_RESPONSE_PACKET_DATA_READY;

    if (dbp_response_write (response, dbp_action_request_writer) != SUCCESS)
    {
        return (DBP_RESPONSE_ERROR_WRITE);
    }

    return (SUCCESS);
}

int
dbp_prehook_request (dbp_request_s *request)
{
    if (!filemgmt_file_exists (
          request->file_info.file_name, request->file_info.real_file_name, NULL))
    {
        return (DBP_RESPONSE_FILE_NOT_FOUND);
    }

    if (request->header_info.data_length != 0)
    {
        return (DBP_RESPONSE_DATA_NONE_NEEDED);
    }
    request->data_write_confirm = TRUE;
    return (SUCCESS);
}
