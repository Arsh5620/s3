#include "protocol.h"

int dbp_prehook_notification(dbp_request_s *request)
{
	if (request->header_info.data_length > 0 
		&& request->header_info.data_length < FILE_BUFFER_LENGTH)
	{
		return(SUCCESS);
	} 
	else
	{
		return(DBP_RESPONSE_NOTIFY_TOOBIG);
	} 
}

int dbp_posthook_notification(dbp_request_s *request, dbp_response_s *response)
{
	FILE *file  = 
		fopen(request->temp_file.name.address, FILE_MODE_READBINARY);

	file_reader_s reader    = file_reader_init(file);
	file_reader_fill(&reader, 0, request->header_info.data_length);

	output_handle(OUTPUT_HANDLE_BOTH, LOGGER_LEVEL_INFO
		, NOTIFICATION_GENERAL
		, (int)reader.readlength, (char*)reader.buffer);

	file_reader_close(&reader);
	return(SUCCESS);
}