#include "../protocol.h"

// this function does not delete the actual file, but only removes link in db
// so the file is still recoverable, unless an overwrite to the same name
int
s3_preprocess_delete (s3_request_s *request)
{
    if (request->header_info.data_length)
    {
        return (DBP_RESPONSE_UNEXPECTED_DATA_FROM_CLIENT);
    }

    if (filemgmt_remove_meta (request->file_name.file_name) != SUCCESS)
    {
        return (DBP_RESPONSE_SERVER_INTERNAL_ERROR);
    }
    return (DBP_RESPONSE_SUCCESS);
}

int
s3_postprocess_delete (s3_request_s *request, s3_response_s *response)
{
    return (DBP_RESPONSE_SUCCESS); // this function is not handled
}
