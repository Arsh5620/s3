#include "protocol.h"
#include "../files/filemgmt.h"

int dbp_prehook_create(dbp_request_s *request)
{
	dbp_protocol_attribs_s attribs = request->attribs;
	
	if(STRING_ISEMPTY(attribs.file_name))
	{
		return(DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	if (filemgmt_file_exists(attribs.file_name))
	{
		return(DBP_RESPONSE_FILE_EXISTS_ALREADY);
	}

	request->working_file_name	= file_path_concat
		(STRING(FILEMGMT_FOLDER_NAME), attribs.file_name, FALSE);
	return(SUCCESS);
}

int dbp_posthook_create(dbp_request_s *request, dbp_response_s *response)
{
	// so now at this time, we can assume that the file does not exists already
	// and that the data has been downloaded to a temp file.
	// all we need to do is a simple file move. 

	string_s destination	= request->working_file_name;
	string_s source	= request->temp_file.name;

	if (file_dir_mkine(FILEMGMT_FOLDER_NAME) != FILE_DIR_EXISTS)
	{
		return (DBP_RESPONSE_GENERAL_DIR_ERROR);
	}

	my_list_s path_list	= path_parse(destination).path_list;
	char *path	= path_construct(path_list, TRUE);
	char *path2 = path_construct(path_list, FALSE);
	int result	= path_mkdir_recursive(path);
	m_free(path, MEMORY_FILE_LINE);
	if (result != SUCCESS)
	{
		return (DBP_RESPONSE_GENERAL_DIR_ERROR);
	}
	string_s s1;
	s1.address	= path2;
	s1.length	= strlen(path2);
	if (filemgmt_rename_file(s1, source)
		|| filemgmt_file_add(s1))
	{
		return(DBP_RESPONSE_GENERAL_FILE_ERROR);
	}
	return (SUCCESS);
}