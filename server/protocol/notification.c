#include "protocol.h"

int dbp_notification_prehook(packet_info_s *info)
{
	if(info->header.data_length > 0 
		&& info->header.data_length < FILE_READER_BUFFERLENGTH){
		return(SUCCESS);
	} else return(DBP_CONN_NOTIFICATION_SIZE);
}

int dbp_notification_posthook(packet_info_s *info)
{
	FILE *file  = 
		fopen(info->data_written.filename.address, FILE_MODE_READONLY);
	file_reader_s reader    = file_init_reader(file);
	
	file_reader_fill(&reader, 0, info->header.data_length);

	error_handle(ERRORS_HANDLE_STDOLOG, LOGGER_INFO
		, NOTIFICATION_GENERAL_IS
		, (int)reader.readlength, (char*)reader.buffer);

	file_close_reader(&reader);
	return(SUCCESS);
}