#include "protocol.h"
#include <openssl/sha.h>

int dbp_request_data(dbp_protocol_s *protocol, dbp_request_s *request)
{
	if (dbp_request_data_headers(protocol, request) == SUCCESS)
	{
		request->temp_file	= dbp_file_download(request);
	} 
	else
	{
		return(DBP_RESPONSE_CORRUPTED_DATA_HEADERS);
	}
	return(DBP_RESPONSE_SUCCESS);
}

/*
 * headers for data has 2 byte magic, 0xd0,0xd1 and 6 byte length
 */
int dbp_request_data_headers(dbp_protocol_s *protocol, dbp_request_s *request)
{
	network_data_atom_s data	= network_read_long(&protocol->connection);
	if (((data._u.long_t >> (6*8))&0xFFFF) != 0xd0d1)
	{
		return(FAILED);
	}
	if (request->header_info.data_length 
		!= ((data._u.long_t) & 0x0000FFFFFFFFFFFF))
	{
		return(FAILED);
	}
	return(SUCCESS);
}

void dbp_file_hash_sha1(file_write_s *writer, network_data_s data, boolean fin)
{
	if (writer->hash_buffer == NULL)
	{
		writer->hash_buffer	= m_calloc(FILE_BUFFER_LENGTH, MEMORY_FILE_LINE);
		writer->hash_size	= (FILE_BUFFER_LENGTH);
		writer->hash_index	= 0;
		// default element count is 10, and each element is 20 in length
		writer->hash_list	= my_list_new(10, 20); 
	}
	
	ulong write	= (writer->hash_size - writer->hash_index);
	ulong actual_write	= data.data_length;

	if (write > actual_write)
	{
		write	= actual_write;
	}

	// first lets copy the data required.
	if (write)
	{
		memcpy(writer->hash_buffer + writer->hash_index, data.data_address, write);
	}

	writer->hash_index += write;
	if (writer->hash_index	== writer->hash_size || (fin && writer->hash_index))
	{
		char sha1[20];
		unsigned char *digest	= SHA1((unsigned char*)writer->hash_buffer
			, writer->hash_size, (unsigned char*)sha1);

		my_list_push(&writer->hash_list, (char*)digest);

		ulong diff = actual_write - write;
		memcpy(writer->hash_buffer, data.data_address, diff);
		memset(writer->hash_buffer + diff, NULL_ZERO, writer->hash_size - diff);
		writer->hash_index	= diff;
	}
}

int dbp_file_hash_writesha1(char *file_name, my_list_s list)
{
	FILE *file	= fopen(file_name, FILE_MODE_WRITEBINARY);
	char *sha1_hex	= m_malloc(41, MEMORY_FILE_LINE);
	for (ulong i = 0; i < list.count; ++i)
	{
		// considering our list size is 20 bytes constant. 
		// to translate this into hex values, it will be 40 bytes

		unsigned char *digest	=  (unsigned char*)my_list_get(list, i);
		for (size_t j = 0; j < 20; j++)
		{
			snprintf(sha1_hex + (j * 2), 3, "%02X", *(digest + j));
		}
		*(sha1_hex + 40)	= '\n';

		if (fwrite(sha1_hex, 1, 41, file) != 41)
		{
			return (FAILED);
		}
	}
	fflush(file);
	return (SUCCESS);
}

file_write_s dbp_file_download(dbp_request_s *request)
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
	int download_status   = file_download(temp
		, &protocol->connection, &fileinfo, dbp_file_hash_sha1);

	// assert(request->working_file_name.address); // its a bug if value not set

	char *hash_file_name;
	length	= strings_sprintf(&hash_file_name, "%s/temp-(%d).sha1"
		, DBP_TEMP_DIR, counter);
	
	request->temp_hash_file.address	= hash_file_name;
	request->temp_hash_file.length	= length;

	dbp_file_hash_writesha1(hash_file_name, fileinfo.hash_list);

	m_free(hash_file_name, MEMORY_FILE_LINE); // will log out a warning
	m_free(fileinfo.hash_buffer, MEMORY_FILE_LINE);

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

int dbp_file_setup_environment()
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