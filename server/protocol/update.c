#include "protocol.h"
#include "../files/filemgmt.h"

int dbp_prehook_update(dbp_request_s *request)
{
	dbp_protocol_attribs_s attribs = request->attribs;

	if(STRINGS_EMPTY(attribs.file_name) 
		|| STRINGS_EMPTY(attribs.folder_name))
	{
		return(DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	string_s file = file_path_concat(STRING_S(FILEMGMT_FOLDER_NAME)
		, attribs.folder_name, attribs.file_name);

	FILE *file_f	= fopen(file.address, FILE_MODE_READONLY);

	if (!(filemgmt_file_exists(&attribs.folder_name, &attribs.file_name)
		&& file_f != NULL))
	{
		return(DBP_RESPONSE_FILE_NOT_FOUND);
	}
	
	struct stat file_stats	= file_read_stat(file_f);
	fclose(file_f);

	if (attribs.update_at > -1 && attribs.update_at <= file_stats.st_size)
	{
		// then everything is good, we can update the file. 
		return(SUCCESS);
	}
	else 
	{
		return(DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS);
	}
}

int dbp_posthook_update(dbp_request_s *request, dbp_response_s *response)
{
	return(SUCCESS);
}