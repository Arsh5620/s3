#include "protocol.h"
#include "../files/filemgmt.h"
#include <sys/types.h>

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
	hash_table_bucket_s	data;
	data.key.address	= DBP_KEY_FILENAME;
	data.key_len		= sizeof(DBP_KEY_FILENAME) - 1;
	data.value.address	= file.address;
	data.value_len		= file.length;

	hash_table_add(&request->additional_data, data);

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
	hash_input_u key_name	= {
		.address = DBP_KEY_FILENAME
	};
	ulong key_length	= sizeof(DBP_KEY_FILENAME) - 1;

	hash_table_bucket_s	file_name	= hash_table_get
		(request->additional_data, key_name, key_length);

	char *file_name_s	= file_name.value.address;
	if (file_name_s == NULL)
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}

	if (truncate(file_name.value.address, request->attribs.update_at))
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}

	int result	= file_append(file_name_s
		, request->temp_file.filename.address
		, request->header_info.data_length);

	if (result != FILE_SUCCESS)
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}
	return(SUCCESS);
}