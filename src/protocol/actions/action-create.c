#include "protocol/protocol.h"

int
s3_preprocess_create (s3_request_s *request)
{
    boolean append_exists = FALSE;
    s3_attrib_get (request->header_table, S3_ATTRIBNAME_APPEND, NULL, &append_exists);

    if (filemgmt_file_exists (request->file_name.file_name, request->file_name.real_file_name, NULL) && append_exists == FALSE) {
        return S3_RESPONSE_FILE_EXISTS_ALREADY;
    }

    request->additional_data = (char *) (long) append_exists;

    return SUCCESS;
}

int
s3_postprocess_create (network_s *network)
{
    // so now at this time, we can assume that the file does not exists already
    // and that the data has been downloaded to a temp file.
    // all we need to do is a simple file move.

    s3_request_s *request = network->request;
    string_s dest = request->file_name.real_file_name;
    string_s source = request->file_name.temp_file_name;

    struct stat file_stats;
    if (request->additional_data != NULL && filemgmt_file_exists (request->file_name.file_name, dest, &file_stats)) {
        file_append (dest.address, source.address, file_stats.st_size, request->header_info.data_length);
    } else if (filemgmt_rename_file (dest, source) == FAILED ||
               filemgmt_file_add (request->file_name.file_name,
                                  STRING (network->sha256_hex),
                                  network->request->header_info.data_length) == FAILED) {
        return S3_RESPONSE_SERVER_FILE_ERROR;
    }
    return SUCCESS;
}