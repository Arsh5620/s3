#include "protocol.h"

file_write_s dbp_download_file(dbp_request_s *request)
{
	static int counter = 0;

	char *temp_file;
	file_write_s fileinfo  =  {0};
	fileinfo.size = request->header_info.data_length;

	long length  = strings_sprintf(&temp_file, DBP_TEMP_FORMAT
		, DBP_TEMP_DIR
		, ++counter);
	
	FILE *temp  = fopen(temp_file, FILE_MODE_WRITEONLY);

	if (temp == NULL || length <= 0) 
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_DOWNLOAD_FILE_NOOPEN);
		fileinfo.size   = -1;
		return(fileinfo);
	}
	fileinfo.filename.address   = temp_file;
	fileinfo.filename.length    = length;

	clock_t starttime = clock();

	dbp_protocol_s *protocol	= (dbp_protocol_s*)request->instance;
	int download_status   = 
		file_download(temp, &protocol->connection, &fileinfo);

	clock_t endtime = clock();

	double time_elapsed = 
		(((double)(endtime - starttime)) / CLOCKS_PER_SEC) * 1000;

	// file download size in bits/second
	double speed = (((double)fileinfo.size / 1024 / 128) 
		* (1000 / time_elapsed));

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_DOWNLOAD_COMPLETE
		, request->attribs.file_name.length 
		, request->attribs.file_name.address 
		, fileinfo.size , download_status
		, time_elapsed , speed);

	fclose(temp);
	return(fileinfo);
}

int dbp_setup_environment()
{
	// first make sure the temporary file directory exists. 
	int result  = file_dir_mkine(DBP_TEMP_DIR);
	if(result != FILE_DIR_EXISTS)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, PROTOCOL_SETUP_ENV_DIR_PERMISSIONS
			, DBP_TEMP_DIR);
		return(FAILED);
	}
	return(SUCCESS);
}
