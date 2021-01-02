#include "../protocol.h"
#include <sys/types.h>

int
dbp_prehook_update (dbp_request_s *request)
{
    struct stat file_stats = {0};

    if (!filemgmt_file_exists (
          request->file_info.file_name, request->file_info.real_file_name, &file_stats))
    {
        return (DBP_RESPONSE_FILE_NOT_FOUND);
    }

    request->additional_data = m_malloc (sizeof (dbp_action_update_s), MEMORY_FILE_LINE);

    dbp_action_update_s *update_attribs = (dbp_action_update_s *) request->additional_data;

    int error;
    update_attribs->update_at
      = data_get_kvpair (request->header_list, request->header_table, DBP_ATTRIB_UPDATEAT, &error)
          .value_length;

    if (error != SUCCESS)
    {
        return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
    }

    update_attribs->trim
      = (data_get_kvpair (request->header_list, request->header_table, DBP_ATTRIB_UPDATETRIM, &error)
          .value_length != FALSE);

    if (error != SUCCESS)
    {
        return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
    }

    if (update_attribs->update_at >= 0 && update_attribs->update_at <= file_stats.st_size)
    {
        return (SUCCESS);
    }
    else
    {
        return (DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS);
    }
}

int
dbp_posthook_update (dbp_request_s *request, dbp_response_s *response)
{
    dbp_action_update_s *update_attribs = (dbp_action_update_s *) request->additional_data;

    string_s real_file = request->file_info.real_file_name;
    if (update_attribs->trim && truncate (real_file.address, update_attribs->update_at))
    {
        return (DBP_RESPONSE_SERVER_INTERNAL_ERROR);
    }

    int result = file_append (
      real_file.address,
      request->file_info.temp_file_name.address,
      update_attribs->update_at,
      request->header_info.data_length);

    // TODO free memory used.
    if (result != FILE_SUCCESS)
    {
        return (DBP_RESPONSE_SERVER_INTERNAL_ERROR);
    }
    return (SUCCESS);
}