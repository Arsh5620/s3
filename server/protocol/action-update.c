#include "protocol.h"
#include "../files/filemgmt.h"
#include <sys/types.h>

int dbp_prehook_update(dbp_request_s *request)
{
	string_s file_name	= request->file_info.real_file_name;
	FILE *file	= fopen(file_name.address, FILE_MODE_READBINARY);
	
	if (!(filemgmt_file_exists(request->file_info.file_name) && file != NULL))
	{
		return (DBP_RESPONSE_FILE_NOT_FOUND);
	}
	
	struct stat file_stats	= file_stat(file);
	fclose(file);

	long update_at	= 0;
	if (data_get_and_convert(request->data_result, DBP_ATTRIB_UPDATEAT
		, CONFIG_TYPE_LONG, (char*)&update_at, sizeof(long)))
	{
		return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	if (update_at > -1 && update_at <= file_stats.st_size)
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
	boolean trunc	= 0;
	if (data_get_and_convert(request->data_result, DBP_ATTRIB_UPDATETRIM
		, CONFIG_TYPE_BOOLEAN, (char*)&trunc, sizeof(boolean)))
	{
		return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	ulong update_at	= 0;
	if (data_get_and_convert(request->data_result, DBP_ATTRIB_UPDATEAT
		, CONFIG_TYPE_LONG, (char*)&update_at, sizeof(long)))
	{
		return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
	}

	string_s real_file	= request->file_info.real_file_name;
	if (trunc && truncate(real_file.address, update_at))
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}

	int result	= file_append(real_file.address
		, request->file_info.temp_file_name.address
		, update_at
		, request->header_info.data_length);
	
	//TODO free memory used.
	if (result != FILE_SUCCESS)
	{
		return(DBP_RESPONSE_GENERAL_SERVER_ERROR);
	}
	return(SUCCESS);
}