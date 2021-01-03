#include "../protocol.h"

int
s3_preprocess_create (s3_request_s *request)
{
    if (filemgmt_file_exists (
          request->file_name.file_name, request->file_name.real_file_name, NULL))
    {
        return (S3_RESPONSE_FILE_EXISTS_ALREADY);
    }
    return (SUCCESS);
}

int
s3_postprocess_create (s3_request_s *request, s3_response_s *response)
{
    // so now at this time, we can assume that the file does not exists already
    // and that the data has been downloaded to a temp file.
    // all we need to do is a simple file move.

    string_s dest = request->file_name.real_file_name;
    string_s source = request->file_name.temp_file_name;

    string_s sha1_dest = request->file_name.real_hash_file_name;
    string_s sha1_src = request->file_name.temp_hash_file_name;

    if (
      filemgmt_rename_file (dest, source) || filemgmt_rename_file (sha1_dest, sha1_src)
      || filemgmt_file_add (request->file_name.file_name))
    {
        return (S3_RESPONSE_SERVER_FILE_ERROR);
    }
    return (SUCCESS);
}