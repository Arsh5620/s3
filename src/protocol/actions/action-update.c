#include "protocol/protocol.h"

#include <sys/types.h>

int
s3_preprocess_update (s3_request_s *request)
{
    struct stat file_stats = {0};

    if (!filemgmt_file_exists (request->file_name.file_name, request->file_name.real_file_name, &file_stats)) {
        return S3_RESPONSE_FILE_NOT_FOUND;
    }

    request->additional_data = m_malloc (sizeof (s3_action_update_s));

    s3_action_update_s *update_attribs = (s3_action_update_s *) request->additional_data;

    size_t update_position, trim;
    boolean update_position_found, trim_found;

    s3_attrib_get (request->header_table, S3_ATTRIBNAME_UPDATEAT, &update_position, &update_position_found);
    s3_attrib_get (request->header_table, S3_ATTRIBNAME_UPDATETRIM, &trim, &trim_found);

    if (update_position_found == FALSE || trim_found == FALSE) {
        return S3_RESPONSE_ATTRIB_VALUE_INVALID;
    }

    update_attribs->update_at = update_position;
    update_attribs->trim = trim != FALSE;

    if (update_attribs->update_at >= 0 && update_attribs->update_at <= file_stats.st_size) {
        return SUCCESS;
    } else {
        return S3_RESPONSE_FILE_UPDATE_OUTOFBOUNDS;
    }
}

int
s3_postprocess_update (network_s *network)
{
    s3_request_s *request = network->request;
    s3_action_update_s *update_attribs = (s3_action_update_s *) request->additional_data;

    string_s real_file = request->file_name.real_file_name;
    if (update_attribs->trim && truncate (real_file.address, update_attribs->update_at)) {
        return S3_RESPONSE_SERVER_INTERNAL_ERROR;
    }

    int result = file_append (
      real_file.address, request->file_name.temp_file_name.address, update_attribs->update_at, request->header_info.data_length);

    // TODO free memory used.
    if (result != FILE_SUCCESS) {
        return S3_RESPONSE_SERVER_INTERNAL_ERROR;
    }
    return SUCCESS;
}