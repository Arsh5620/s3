#include "protocol.h"

int dbp_prehook_notification(dbp_request_s *request)
{
	if (request->header_info.data_length > 0 
		&& request->header_info.data_length < FILE_READER_BUFFERLENGTH)
	{
		return(SUCCESS);
	} 
	else
	{
		request->warn	= DBP_CONNECTION_WARN_EMPTY;
		return(DBP_CONNECTION_NOERROR);
	} 
}

int dbp_posthook_notification(dbp_request_s *request, dbp_response_s *response)
{
	FILE *file  = 
		fopen(request->temp_file.filename.address, FILE_MODE_READONLY);

	file_reader_s reader    = file_init_reader(file);
	file_reader_fill(&reader, 0, request->header_info.data_length);

	error_handle(ERRORS_HANDLE_STDOLOG, LOGGER_LEVEL_INFO
		, NOTIFICATION_GENERAL
		, (int)reader.readlength, (char*)reader.buffer);

	file_close_reader(&reader);
	return(SUCCESS);
}