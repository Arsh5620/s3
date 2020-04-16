#include "protocol.h"
#include "../files/filemgmt.h"

int dbp_prehook_update(dbp_request_s *request)
{
	dbp_protocol_attribs_s attribs = request->attribs;

	if(STRINGS_EMPTY(attribs.file_name) 
		|| STRINGS_EMPTY(attribs.folder_name))
	{
		return(DBP_CONNECTION_WARN_ATTRIB_VALUE_INVALID);
	}

	if (!filemgmt_file_exists(&attribs.folder_name, &attribs.file_name))
	{
		return(DBP_CONNECTION_WARN_FILE_NOT_FOUND);
	}

	string_s file = file_path_concat(STRING_S(FILEMGMT_FOLDER_NAME)
		, attribs.folder_name, attribs.file_name);
	struct stat file_stats	= file_read_stat(file.address);
	
}

int dbp_posthook_update(dbp_request_s *request, dbp_response_s *response)
{
	
}