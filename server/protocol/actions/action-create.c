#include "../protocol.h"

int
dbp_prehook_create (dbp_request_s *request)
{
    if (filemgmt_file_exists (
          request->file_info.file_name, request->file_info.real_file_name, NULL))
    {
        return (DBP_RESPONSE_FILE_EXISTS_ALREADY);
    }
    return (SUCCESS);
}

int
dbp_posthook_create (dbp_request_s *request, dbp_response_s *response)
{
    // so now at this time, we can assume that the file does not exists already
    // and that the data has been downloaded to a temp file.
    // all we need to do is a simple file move.

    string_s dest = request->file_info.real_file_name;
    string_s source = request->file_info.temp_file_name;

    string_s sha1_dest = request->file_info.real_hash_file_name;
    string_s sha1_src = request->file_info.temp_hash_file_name;

    if (
      filemgmt_rename_file (dest, source) || filemgmt_rename_file (sha1_dest, sha1_src)
      || filemgmt_file_add (request->file_info.file_name))
    {
        return (DBP_RESPONSE_GENERAL_FILE_ERROR);
    }
    return (SUCCESS);
}