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
	
	if (filemgmt_file_add(&attribs.folder_name, &attribs.file_name) == FAILED)
	{
		return(DBP_CONNECTION_GENERAL_SERVER_ERROR);
	}
	return(SUCCESS);
}

int dbp_posthook_create(dbp_request_s *request, dbp_response_s *response)
{
	// so now at this time, we can assume that the file does not exists already
	// and that the data has been downloaded to a temp file.
	// all we need to do is a simple file move. 

	dbp_protocol_attribs_s attribs	= request->attribs;
	string_s destination = file_path_concat(STRING_S(FILEMGMT_FOLDER_NAME)
		, attribs.folder_name, attribs.file_name);
	string_s source	= request->temp_file.filename;

	if (filemgmt_rename_file(destination, source) != SUCCESS) 
	{
		return (DBP_CONNECTION_GENERAL_SERVER_ERROR);
	}
	return (DBP_CONNECTION_NOWARN);
}