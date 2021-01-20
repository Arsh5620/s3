#include "../protocol.h"

int
s3_preprocess_dirlist (s3_request_s *request)
{
    int error = 0;
    string_s folder_name
      = data_get_string_s (request->header_list, request->header_table, S3_ATTRIB_DIRNAME, &error);
    if (filemgmt_folder_exists (folder_name) == FALSE)
    {
        return (S3_RESPONSE_FOLDER_NOT_FOUND);
    }

    if (request->header_info.data_length != 0)
    {
        return (S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT);
    }
    
    return (SUCCESS);
}
