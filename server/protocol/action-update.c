#include "protocol.h"
#include "../files/filemgmt.h"
#include <sys/types.h>

int dbp_prehook_update(dbp_request_s *request)
{
	struct stat file_stats = {0};
	
	if (!filemgmt_file_exists(request->file_info.file_name
		, request->file_info.real_file_name, &file_stats))
	{
		return (DBP_RESPONSE_FILE_NOT_FOUND);
	}

	request->additional_data	= 
		m_malloc(sizeof(dbp_action_update_s), MEMORY_FILE_LINE);

	dbp_action_update_s *update_attribs = 
		(dbp_action_update_s*) request->additional_data;

	if (data_get_and_convert(request->data_result, DBP_ATTRIB_UPDATEAT
		, CONFIG_TYPE_LONG, (char*)&update_attribs->update_at, sizeof(long)))
	{
		return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	if (data_get_and_convert(request->data_result, DBP_ATTRIB_UPDATETRIM
		, CONFIG_TYPE_BOOLEAN, (char*)&update_attribs->trim, sizeof(boolean)))
	{
		return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	if (update_attribs->update_at > -1 
		&& update_attribs->update_at <= file_stats.st_size)
	{
		return(SUCCESS);
	}
	else 
	{
		return(DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS);
	}
}

int dbp_posthook_update(dbp_request_s *request, dbp_response_s *response)
{
	dbp_action_update_s *update_attribs = 
		(dbp_action_update_s*) request->additional_data;

	string_s real_file	= request->file_info.real_file_name;
	if (update_attribs->trim 
		&& truncate(real_file.address, update_attribs->update_at))
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}

	int result	= file_append(real_file.address
		, request->file_info.temp_file_name.address
		, update_attribs->update_at
		, request->header_info.data_length);
	
	//TODO free memory used.
	if (result != FILE_SUCCESS)
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}
	return(SUCCESS);
}