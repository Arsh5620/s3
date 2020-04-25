#include "protocol.h"
#include "../files/filemgmt.h"

int dbp_prehook_create(dbp_request_s *request)
{
	dbp_protocol_attribs_s attribs = request->attribs;
	
	if(STRINGS_EMPTY(attribs.file_name))
	{
		return(DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	if (filemgmt_file_exists(attribs.file_name))
	{
		return(DBP_RESPONSE_FILE_EXISTS_ALREADY);
	}

	request->working_file_name	= file_path_concat
		(STRING_S(FILEMGMT_FOLDER_NAME), attribs.file_name);
	return(SUCCESS);
}

int dbp_posthook_create(dbp_request_s *request, dbp_response_s *response)
{
	// so now at this time, we can assume that the file does not exists already
	// and that the data has been downloaded to a temp file.
	// all we need to do is a simple file move. 

	dbp_protocol_attribs_s attribs	= request->attribs;
	string_s destination = request->working_file_name;
	string_s source	= request->temp_file.filename;

	if (file_dir_mkine(FILEMGMT_FOLDER_NAME) != FILE_DIR_EXISTS
		|| paths_mkdir_recursive(paths_parse(destination)) != SUCCESS)
	{
		return (DBP_RESPONSE_GENERAL_DIR_ERROR);
	}

	if (filemgmt_rename_file(destination, source)
		|| filemgmt_file_add(attribs.file_name))
	{
		return(DBP_RESPONSE_GENERAL_FILE_ERROR);
	}
	return (SUCCESS);
}