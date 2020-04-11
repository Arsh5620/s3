#include <string.h>
#include <time.h>
#include <unistd.h>
#include "protocol.h"

// the function will attempt to find the most common attributes
// that are mostly used by all the "actions", attributes that 
// are specific (one-to-one) to an "action" is not handled here. 

static dbp_header_keys_s attribs[] = 
{
	DBP_STRINGKEY("action", DBP_ATTRIB_ACTION)
	, DBP_STRINGKEY("crc", DBP_ATTRIB_CRC)
	, DBP_STRINGKEY("filename", DBP_ATTRIB_FILENAME)
	, DBP_STRINGKEY("folder", DBP_ATTRIB_FOLDER)
};

static dbp_header_keys_s actions[] = 
{
	DBP_STRINGKEY("create", DBP_ACTION_CREATE)
	, DBP_STRINGKEY("notification", DBP_ACTION_NOTIFICATION)
	, DBP_STRINGKEY("request", DBP_ACTION_REQUEST)
	, DBP_STRINGKEY("update", DBP_ACTION_UPDATE)
};

static enum dbp_attribs_enum dbp_call_asserts[]
[sizeof(attribs) / sizeof(enum dbp_attribs_enum)] = {
	// ACTION_CREATE
	{DBP_ATTRIB_ACTION, DBP_ATTRIB_CRC, DBP_ATTRIB_FILENAME, DBP_ATTRIB_FOLDER}
	, {DBP_ATTRIB_ACTION, 0, 0} // ACTION_NOTIFICATION
	, {DBP_ATTRIB_ACTION, 0, 0} // ACTION_REQUEST
	, {DBP_ATTRIB_ACTION, 0, 0} // ACTION_UPDATE
};

struct config_parse attribs_parse[] = {
	STRUCT_CONFIG_PARSE("action", DBP_ATTRIB_ACTION
		, dbp_protocol_attribs_s, file_name, CONFIG_TYPE_STRING_S)
	, STRUCT_CONFIG_PARSE("crc", DBP_ATTRIB_CRC
		, dbp_protocol_attribs_s, file_name, CONFIG_TYPE_STRING_S)
	, STRUCT_CONFIG_PARSE("filename", DBP_ATTRIB_FILENAME
		, dbp_protocol_attribs_s, file_name, CONFIG_TYPE_STRING_S)
	, STRUCT_CONFIG_PARSE("folder", DBP_ATTRIB_FOLDER
		, dbp_protocol_attribs_s, file_name, CONFIG_TYPE_STRING_S)
};

size_t dbp_header_code_compare(void *memory, char *str, size_t strlen)
{
	dbp_header_keys_s *s    = (dbp_header_keys_s*) memory;

	int cmp = memcmp(str, s->string, strlen); // order does matter it is a - b

	if (cmp == 0 && s->strlen > strlen)
	{
		cmp--;
	}
	else if (cmp == 0 && s->strlen < strlen)
	{
		cmp++;
	}
	return(cmp);
}

/** 
 * This function will initialize the connection and everything
 * It will however exit for any error that is not recoverable.
 */
dbp_protocol_s dbp_connection_initialize_sync(unsigned short port)
{
	/* variables are inited right to left first */
	dbp_protocol_s protocol = { 0 };
	protocol.logs = logs_open();

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_LOG_INIT_COMPLETE);
		
	protocol.connection = network_connect_init_sync(port);

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_NETWORK_SBS_INIT);
		
	if (database_init(DBP_CONFIG_FILENAME) == MYSQL_SUCCESS) 
	{
		protocol.init_complete = TRUE;
	} 
	else 
	{
		error_handle(ERRORS_HANDLE_LOGS
			, LOG_LEVEL_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC
			, SERVER_DATABASE_FAILURE)
			, PROTOCOL_MYSQL_FAILED_CONNECT);
	}
	return(protocol);
}

void dbp_accept_connection_loop(dbp_protocol_s *protocol)
{
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_NETWORK_WAIT_CONNECT);
	
	int connection_status	= 0;

	while (connection_status = 
		network_connect_accept_sync(&(protocol->connection))
		, connection_status == SUCCESS)
	{ 
		struct sockaddr_in client	= protocol->connection.client_socket;
		char *client_ip		= inet_ntoa(client.sin_addr);
		ushort client_port	= ntohs(client.sin_port);

		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, PROTOCOL_NETWORK_CLIENT_CONNECT
			, client_ip, client_port); 

		int shutdown	= DBP_CONNECTION_SHUTDOWN_FLOW;

		for(enum dbp_errors_enum error = 0; error == DBP_CONNECTION_NOERROR;)
		{
			error	= dbp_protocol_nextrequest(protocol);
			printf("dbp_next returned value: %d\n", error);
			// dbp_handle_errors(error, &shutdown);
			// dbp_handle_warns(protocol->current_request->warn);
		}

		dbp_shutdown_connection(*protocol, shutdown);
	}
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_SERVER_SHUTDOWN);
}

int dbp_handle_errors(enum dbp_errors_enum error, int *shutdown)
{
	if(error != DBP_CONNECTION_NOERROR)
		*shutdown	= DBP_CONNECTION_SHUTDOWN_CORRUPTION;
	return(0);
}

int dbp_handle_warns(enum dbp_errors_enum warn)
{
	if(warn == DBP_CONNECTION_WARN)
	{
		printf("error message.");
	}
	return(0);
}

void dbp_shutdown_connection(dbp_protocol_s protocol
	, enum dbp_shutdown_enum type)
{
	if(close(protocol.connection.client) == 0)
	{
		char *reason;
		switch (type) 
		{
		case DBP_CONNECTION_SHUTDOWN_FLOW:
			reason	= PROTOCOL_SHUTDOWN_REASON_FLOW;
			break;
		case DBP_CONNECTION_SHUTDOWN_CORRUPTION:
			reason	= PROTOCOL_SHUTDOWN_REASON_CORRUPT;
			break;
		default: 
			reason	= PROTOCOL_SHUTDOWN_REASON_UNKNOWN;
			break;
		}
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_CLIENT_CONNECT_ABORTED, reason);
	}
}

void dbp_cleanup(dbp_protocol_s protocol)
{
	close(protocol.connection.client);
	close(protocol.connection.server);
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DBP_CONNECTION_SHUTDOWN_CLEANUP);
	logs_close();
}

ulong dbp_request_readheaders(dbp_protocol_s protocol, dbp_request_s *request)
{
	network_data_atom_s header_read	= network_read_long(&protocol.connection);
	dbp_header_s header_1 = {0};

	long magic	= header_read._u.long_t;
	header_1.data_length	= dbp_data_length(magic);
	header_1.header_length	= dbp_header_length(magic);
	header_1.magic	= dbp_header_magic(magic);

	request->header_info = header_1;
	
	if (header_1.magic != DBP_PROTOCOL_MAGIC)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_ABORTED_CORRUPTION
			, header_1.magic);
		request->warn	= DBP_CONNECTION_WARN_CORRUPTION;
		return(DBP_CONNECTION_WARN);
	}

	network_data_s header_raw	= 
		network_read_stream(&protocol.connection, header_1.header_length);
	
	if (header_raw.error_code)
	{
		error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_READ_HEADERS_FAILED);
		request->warn	= DBP_CONNECTION_WARN_READERROR;
		return(DBP_CONNECTION_WARN);
	}

	lexer_status_s status	= {0};
	my_list_s header_list	= 
		parser_parse(&status, header_raw.data_address, header_raw.data_length);
	
	if (status.errno != 0) 
	{
		/* this means that an error occured while processing the input */
		request->warn	= DBP_CONNECTION_WARN_PARSEERROR;
		return(DBP_CONNECTION_WARN);
	}
	request->header_list	= header_list;
	return(DBP_CONNECTION_NOERROR);
}

enum dbp_actions_enum dbp_read_action(dbp_request_s *request)
{
	key_value_pair_s pair   = {0};
	my_list_s list	= request->header_list;
	if (list.count > 0)
	{
		pair	= *(key_value_pair_s*) my_list_get(list, 0);
	} 
	else 
	{
		request->warn  = DBP_CONNECTION_WARN_EMPTY;
		return(DBP_ACTION_NOTVALID);
	}

	dbp_header_keys_s action	= attribs[0];
	int actionval	= DBP_ACTION_NOTVALID;
	if (memcmp(pair.key, action.string, pair.key_length) == 0) 
	{
		// now here to check the action that the client is requesting.
		actionval	= binary_search(actions
			, sizeof(dbp_header_keys_s)
			, sizeof(actions) / sizeof(dbp_header_keys_s)
			, pair.value , pair.value_length
			, dbp_header_code_compare);
	}

	if (actionval  == DBP_ACTION_NOTVALID)
	{
		request->warn	= DBP_CONNECTION_WARN_ACTION_INVALID;
	}
	return(actionval);
}

void dbp_request_cleanup()
{
	/* TODO:: */
}

ulong dbp_request_error(ulong result)
{
	if (result == DBP_CONNECTION_WARN)
	{
		dbp_request_cleanup();
		return(DBP_CONNECTION_NOERROR);
	}
	else 
	{
		return(result);
	}
}

// this function should be called before dispatching the request
// to make sure that the header contains all the required key:value 
// pairs needed by the called function.
int dbp_list_assert(hash_table_s table, 
	enum dbp_attribs_enum *match, int count)
{
	for (long i=0; i<count; i++)
	{
		enum dbp_attribs_enum attrib	= match[i];
		hash_input_u key	= {.number = attrib};
		hash_table_bucket_s bucket	= hash_table_get(table, key, NULL_ZERO);
		if (bucket.is_occupied == 0)
		{
			return(FALSE);
		}
	}
	return(TRUE);
}


// returns 0 for no-error, any other number for error or conn close request
ulong dbp_protocol_nextrequest(dbp_protocol_s *protocol)
{
	dbp_request_s request	= {0};
	dbp_response_s response	= {0};

	int result	= dbp_request_readheaders(*protocol, &request);
	if (result != DBP_CONNECTION_NOERROR)
	{
		return(dbp_request_error(result));
	}
	
	request.instance	= (char*)protocol;
	request.header_table   = dbp_headers_make_table(request.header_list);

	enum dbp_attribs_enum *asserts = dbp_call_asserts[request.action];
	boolean assert	= dbp_list_assert(request.header_table, asserts
		, sizeof(dbp_call_asserts) / sizeof(dbp_call_asserts[0]));

	if (assert == FALSE)
	{
		return(dbp_request_error(DBP_CONNECTION_WARN_THIN_ATTRIBS));
	}
	
	//right after we have confirmed that all the required attribs
	//are present for the function call to succeed, we can parse them
	//to the structure for dbp_common_attribs_s
	
	dbp_protocol_attribs_s dbp_parsed_attribs	= {0};

	config_read_all(request.header_list
		, attribs_parse
		, sizeof(attribs_parse) / sizeof(struct config_parse)
		, (char*)&dbp_parsed_attribs);

	result = dbp_action_prehook(&request);

	if (result != DBP_CONNECTION_NOERROR)
	{
		return(dbp_request_error(result));
	}

	if (request.header_info.data_length) 
	{
		if (dbp_setup_environment() == SUCCESS) 
		{
			request.temp_file	= dbp_download_file(&request);
		} 
		else 
		{
			printf("setting up env failed. ");
			return DBP_CONNECTION_ERROR_ENV_FAILED;
		}
	}

	result	= dbp_action_posthook(&request, &response);
	
	if (result != DBP_CONNECTION_NOERROR)
	{
		return(dbp_request_error(result));
	}
	
	result	= dbp_response_write(&response);
	
	if (result != DBP_CONNECTION_NOERROR)
	{
		return(dbp_request_error(result));
	}

	m_free(request.temp_file.filename.address, MEMORY_FILE_LINE);

	return(DBP_CONNECTION_NOERROR);
}

int dbp_response_write(dbp_response_s *response)
{
	// TODO: response write. 
	return(0);
}

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

int dbp_action_posthook(dbp_request_s *request, dbp_response_s *response)
{
	int result = 0;
	switch(request->action)
	{
		case DBP_ACTION_NOTIFICATION:
			result  = dbp_posthook_notification(request, response);
			break;
		case DBP_ACTION_CREATE:
			return(SUCCESS);
			break;
	}
	return(result);
}

int dbp_action_prehook(dbp_request_s *request)
{
	int result = 0;
	switch (request->action)
	{
		case DBP_ACTION_NOTIFICATION:
			result = dbp_prehook_notification(request);
			break;
		case DBP_ACTION_CREATE:
			result = dbp_prehook_create(request);
			break;
	}
	return(result);
}

/*
 * this function will go through the list of the key:value pairs
 * and add those key value pairs to a hash table, while making
 * sure that there are not duplicates, if duplicates are found
 * the key:value pair found later is ignored.
 */
hash_table_s dbp_headers_make_table(my_list_s list)
{
	int count	= list.count;
	hash_table_s table  = hash_table_init(10, 0);

	for (long i=0; i<count; ++i) 
	{
		key_value_pair_s pair	=
			*(key_value_pair_s*) my_list_get(list, i);
		
		int index	= binary_search((void*)attribs
			, sizeof(dbp_header_keys_s)
			, sizeof(attribs) / sizeof(dbp_header_keys_s)
			, pair.key, pair.key_length
			, dbp_header_code_compare);

		// will be -1 if an attribute is not supported (YET!)
		// which is also ignored. 
		if (index != -1) 
		{  
			dbp_header_keys_s attr  = attribs[index];

			hash_table_bucket_s b   = hash_table_get(table
				, (hash_input_u) { .address = attr.string }
				, attr.strlen);

			if (b.is_occupied)
			{
				continue;
			}

			hash_table_bucket_s bucket  = {0};
			bucket.key.number	= attr.attrib_code;
			bucket.value.number	= index;

			hash_table_add(&table, bucket);
		}
	}
	return(table);
}

inline short dbp_header_length(size_t magic)
{
	/* shr by 6 bytes, and multiply by 16 to get header size */
	return(((magic & 0x00FF000000000000) >> (6*8)) * 16);
}

inline char dbp_header_magic(size_t magic)
{
	/* shr by 7 to get the magic byte */
	return((magic & 0xFF00000000000000) >> (7*8));
}

inline size_t dbp_data_length(size_t magic)
{
	/* the last 6 bytes are used to store the size of the data */
	return(magic & 0x0000FFFFFFFFFFFF);
}