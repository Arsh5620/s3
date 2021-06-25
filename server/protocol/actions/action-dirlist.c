#include "../protocol.h"

int
s3_preprocess_dirlist (s3_request_s *request)
{
    size_t length = 0;
    char *folder_name = s3_attrib_get (request->header_table, S3_ATTRIBNAME_DIRNAME, &length);
    if (filemgmt_folder_exists (STRING2(folder_name, length)) == FALSE) {
        return S3_RESPONSE_FOLDER_NOT_FOUND;
    }

    if (request->header_info.data_length != 0) {
        return S3_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT;
    }

    return SUCCESS;
}
