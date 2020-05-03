#include "./protocol.h"
#include "../files/filemgmt.h"

// this function does not delete the actual file, but only removes link in db
// so the file is still recoverable, unless an overwrite to the same name
int dbp_prehook_delete(dbp_request_s *request)
{
	if (request->header_info.data_length)
	{
		return (DBP_RESPONSE_DATA_NONE_NEEDED);
	}

	if (filemgmt_remove_meta(request->file_info.file_name) != SUCCESS)
	{
		return (DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}	
	return (DBP_RESPONSE_SUCCESS);
}

int dbp_posthook_delete(dbp_request_s *request, dbp_response_s *response)
{
	return (DBP_RESPONSE_SUCCESS); // this function is not handled
}
