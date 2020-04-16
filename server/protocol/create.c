#include "protocol.h"
#include "../files/file.h"
#include "../general/defines.h"
#include "../files/filemgmt.h"
#include <time.h>

int dbp_prehook_create(dbp_request_s *request)
{
	dbp_protocol_attribs_s attribs = request->attribs;
	
	if(STRINGS_EMPTY(attribs.file_name) 
		|| STRINGS_EMPTY(attribs.folder_name))
	{
		return(DBP_CONNECTION_WARN_ATTRIB_VALUE_INVALID);
	}

	if (filemgmt_file_exists(&attribs.folder_name, &attribs.file_name))
	{
		return(DBP_CONNECTION_WARN_FILE_EXISTS_ALREADY);
	}
	
	if (filemgmt_file_add(&attribs.folder_name, &attribs.file_name) == SUCCESS)
	{
		return(DBP_CONNECTION_GENERAL_SERVER_ERROR);
	}
	return(SUCCESS);
}