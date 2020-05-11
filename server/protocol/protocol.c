#include "protocol.h"

/** 
 * This function will initialize the connection and everything
 * It will however exit for any error that is not recoverable.
 */
dbp_protocol_s dbp_connection_initialize_sync(unsigned short port)
{
	/* variables are inited right to left first */
	dbp_protocol_s protocol = { 0 };
	protocol.logs = logs_open();

	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_LOG_INIT_COMPLETE);
		
	protocol.connection = network_connect_init_sync(port);

	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_NETWORK_SBS_INIT);
		
	if (database_init(DBP_CONFIG_FILENAME) == MYSQL_SUCCESS
		&& database_table_verify(AUTH_TABLE_CHECK
			, AUTH_TABLE_CHECK, AUTH_TABLE_NAME, auth_binds_setup)	== SUCCESS
		&&  database_table_verify(FILEMGMT_TABLE_CHECK
			, FILEMGMT_TABLE_CHECK, FILEMGMT_TABLE_NAME
			, filemgmt_binds_setup)	== SUCCESS)
	{
		protocol.init_complete = TRUE;
	} 
	else 
	{
		output_handle(OUTPUT_HANDLE_LOGS
			, LOG_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC
			, DATABASE_FAILURE_OTHER)
			, PROTOCOL_MYSQL_FAILED_CONNECT);
	}
	return(protocol);
}

static long profiler	= 0;
const long profiler_exit	= 1000000;

void dbp_connection_accept_loop(dbp_protocol_s *protocol)
{
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_NETWORK_WAIT_CONNECT);
	
	while (network_connect_accept_sync(&protocol->connection), TRUE)
	{ 
		struct sockaddr_in client	= protocol->connection.client_socket;
		char *client_ip		= inet_ntoa(client.sin_addr);
		ushort client_port	= ntohs(client.sin_port);

		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, PROTOCOL_NETWORK_CLIENT_CONNECT
			, client_ip, client_port); 

		int shutdown	= DBP_CONNECTION_SHUTDOWN_FLOW;

		for(int error = 0; error == SUCCESS;)
		{
			if (++profiler >= profiler_exit)
			{
				exit(1);
			}
			
			dbp_response_s response	= {0};
			dbp_request_s request	= {0};
			protocol->current_response	= &response;
			protocol->current_request	= &request;
			response.instance	= (char*)protocol;
			request.instance	= (char*)protocol;
			response.file_info	= &request.file_info;

			error	= dbp_next_request(protocol);
			// printf("dbp_next returned value: %d, ", error);

			error	= dbp_handle_response(&response, error);
			if (error != SUCCESS)
			{
				shutdown	= DBP_CONNECTION_SHUTDOWN_CORRUPTION;
			}
			
			dbp_handle_close(&request, &response);
			// printf("dbp_handle_response value: %d\n", error);
		}

		dbp_connection_shutdown(*protocol, shutdown);
	}
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_SERVER_SHUTDOWN);
}

void dbp_handle_close(dbp_request_s *request, dbp_response_s *response)
{
	m_free(request->header_raw.data_address, MEMORY_FILE_LINE);
	my_list_free(request->header_list);
	my_list_free(response->header_list);
	hash_table_free(request->header_table);
	if (request->additional_data)
	{
		m_free(request->additional_data, MEMORY_FILE_LINE);
		request->additional_data	= NULL_ZERO;
	}
	M_FREE(request->file_info.file_name.address);
	M_FREE(request->file_info.real_hash_file_name.address);
	M_FREE(request->file_info.real_file_name.address);
	M_FREE(request->file_info.temp_file_name.address);
	M_FREE(request->file_info.temp_hash_file_name.address);
}

long dbp_handle_response_string(dbp_response_s *response)
{
	string_s link	= {0};

	switch (response->response_code)
	{
	DBP_ASSIGN(link
		, DBP_RESPONSE_DATA_SEND, DBP_RESPONSE_STRING_DATA_SEND);
	
	DBP_ASSIGN(link
		, DBP_RESPONSE_PACKET_OK, DBP_RESPONSE_STRING_PACKET_OK);

	DBP_ASSIGN(link
		, DBP_RESPONSE_ACTION_INVALID, DBP_RESPONSE_STRING_ACTION_INVALID);

	DBP_ASSIGN(link
		, DBP_RESPONSE_HEADER_EMPTY, DBP_RESPONSE_STRING_HEADER_EMPTY);

	DBP_ASSIGN(link
		, DBP_RESPONSE_PARSE_ERROR, DBP_RESPONSE_STRING_PARSE_ERROR);
	
	DBP_ASSIGN(link
		, DBP_RESPONSE_THIN_ATTRIBS, DBP_RESPONSE_STRING_THIN_ATTRIBS);
		
	DBP_ASSIGN(link, DBP_RESPONSE_ATTRIB_VALUE_INVALID
		, DBP_RESPONSE_STRING_ATTIB_VALUE_INVALID);

	DBP_ASSIGN(link, DBP_RESPONSE_FILE_EXISTS_ALREADY
		, DBP_RESPONSE_STRING_FILE_EXISTS_ALREADY);
		
	DBP_ASSIGN(link
		, DBP_RESPONSE_FILE_NOT_FOUND, DBP_RESPONSE_STRING_FILE_NOT_FOUND);
		
	DBP_ASSIGN(link, DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS
		, DBP_RESPONSE_STRING_FILE_UPDATE_OUTOFBOUNDS);
	
	DBP_ASSIGN(link
		, DBP_RESPONSE_CORRUPTED_PACKET, DBP_RESPONSE_STRING_CORRUPTED_PACKET);
	
	DBP_ASSIGN(link, DBP_RESPONSE_CORRUPTED_DATA_HEADERS
		, DBP_RESPONSE_STRING_CORRUPTED_DATA_HEADERS);
	
	DBP_ASSIGN(link
		, DBP_RESPONSE_SETUP_ENV_FAILED,  DBP_RESPONSE_STRING_SETUP_ENV_FAILED);
	
	DBP_ASSIGN(link, DBP_RESPONSE_GENERAL_SERVER_ERROR
		, DBP_RESPONSE_STRING_GENERAL_SERVER_ERROR);

	DBP_ASSIGN(link, DBP_RESPONSE_DATA_NONE_NEEDED
		, DBP_RESPONSE_STRING_DATA_NONE_NEEDED);

	DBP_ASSIGN(link, DBP_RESPONSE_FAILED_AUTHENTICATION
		, DBP_RESPONSE_STRING_FAILED_AUTHENTICATION);

	default:
		link = (string_s){0};
	}

	return (dbp_response_writer_update(response, link));
}

int dbp_handle_response(dbp_response_s *response, enum dbp_response_code code)
{
	if (code == DBP_RESPONSE_SUCCESS)
	{
		return(SUCCESS);
	}
	
	response->response_code	= code;

	if (dbp_response_write(response, dbp_handle_response_string) != SUCCESS)
	{
		return(DBP_RESPONSE_ERROR_WRITE);
	}
	
	if (response->response_code > DBP_RESPONSE_ERRORS)
	{
		return(response->response_code);
	}
	else
	{	
		return(SUCCESS);
	}
}

void dbp_connection_shutdown(dbp_protocol_s protocol
	, enum dbp_shutdown_enum type)
{
	if (shutdown(protocol.connection.client, SHUT_RDWR) == 0)
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
		output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_ERROR
			, PROTOCOL_CLIENT_CONNECT_ABORTED, reason);
	}
}

void dbp_close(dbp_protocol_s protocol)
{
	close(protocol.connection.client);
	close(protocol.connection.server);
	output_handle(OUTPUT_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DBP_CONNECTION_SHUTDOWN_CLEANUP);
	logs_close();
}

int dbp_setupenv(dbp_request_s *request)
{
	string_s client_filename	= {0};
	if (filemgmt_setup_temp_files(&request->file_info) != SUCCESS)
	{
		return (DBP_RESPONSE_CANNOT_CREATE_TEMP_FILE);
	}

	int result	= data_get_and_convert(request->header_list
		, request->header_table
		, DBP_ATTRIB_FILENAME
		, DATA_TYPE_STRING_S
		, (char*)&client_filename
		, sizeof(string_s));

	if (result == SUCCESS)
	{
		if (client_filename.address != NULL && client_filename.length == 0)
		{
			return (DBP_RESPONSE_ATTRIB_VALUE_INVALID);
		}
		result = filemgmt_setup_environment(client_filename
			, &request->file_info);
		
		if (result != SUCCESS)
		{
			return (DBP_RESPONSE_SETUP_ENV_FAILED);
		}

		switch (request->action)
		{
			case DBP_ACTION_CREATE:
			case DBP_ACTION_UPDATE:
			break;

			default:
			{
				return (SUCCESS);
			}
		}

		result	= filemgmt_mkdirs(&request->file_info);
		if (result != SUCCESS)
		{
			return (DBP_RESPONSE_SETUP_ENV_FAILED);
		}
	}
	return (SUCCESS);
}


// returns 0 for no-error, any other number for error or conn close request
ulong dbp_next_request(dbp_protocol_s *protocol)
{
	dbp_request_s *request	= protocol->current_request;
	dbp_response_s *response	= protocol->current_response;

	int result	= dbp_request_read_headers(*protocol, request);
	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}
	
	request->instance	= (char*)protocol;
	request->header_table   = data_make_table(request->header_list
		, attribs, DBP_ATTRIBS_COUNT);

	result	= dbp_request_read_action(request);
	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}

	enum dbp_attribs_enum *asserts = 
		dbp_call_asserts[request->action - DBP_ACTION_CREATE];
	boolean assert	= dbp_attribs_assert(request->header_table, asserts
		, DBP_ATTRIBS_COUNT);

	if (assert == FALSE)
	{
		return(DBP_RESPONSE_THIN_ATTRIBS);
	}

	result	= dbp_auth_transaction(request);

	if (result != DBP_RESPONSE_SUCCESS)
	{
		return (result);
	}
	// TODO

	result	= dbp_setupenv(request);
	if (result != SUCCESS)
	{
		return (result);
	}
	
	result = dbp_action_prehook(request);

	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}

	if (request->header_info.data_length) 
	{
		if (dbp_handle_response(response, DBP_RESPONSE_DATA_SEND) != SUCCESS)
		{
			return(DBP_RESPONSE_ERROR_WRITE);
		}

		result	= dbp_request_data(protocol, request);
		if (result != DBP_RESPONSE_SUCCESS)
		{
			return(result);
		}
	}

	if (request->data_write_confirm == TRUE)
	{
		if (dbp_handle_response(response, DBP_RESPONSE_PACKET_DATA_MORE) != SUCCESS)
		{
			return(DBP_RESPONSE_ERROR_WRITE);
		}
		// client must reply with 0XD0FFFFFFFFFFFFFF to accept data
		int status	= dbp_response_accept_status(response);
		if (status == FAILED)
		{
			return (DBP_RESPONSE_DATA_NOT_ACCEPTED);
		}		
	}

	result	= dbp_action_posthook(request, response);
	
	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}
	
	if (response->response_code != DBP_RESPONSE_PACKET_DATA_READY
		&& dbp_handle_response(response, DBP_RESPONSE_PACKET_OK) != SUCCESS)
	{
		return(DBP_RESPONSE_ERROR_WRITE);
	}

	// TODO still have release resources used by file mgmt.
	return(DBP_RESPONSE_SUCCESS);
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
			result	= dbp_posthook_create(request, response);
			break;
		case DBP_ACTION_UPDATE:
			result	= dbp_posthook_update(request, response);
			break;
		case DBP_ACTION_DELETE:
			result	= dbp_posthook_delete(request, response);
			break;
		case DBP_ACTION_REQUEST:
			result	= dbp_posthook_request(request, response);
			break;
		case DBP_ACTION_SERVER:
			result	= dbp_posthook_serverinfo(request, response);
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
		case DBP_ACTION_UPDATE:
			result	= dbp_prehook_update(request);
			break;
		case DBP_ACTION_DELETE:
			result	= dbp_prehook_delete(request);
			break;
		case DBP_ACTION_REQUEST:
			result	= dbp_prehook_request(request);
			break;
		case DBP_ACTION_SERVER:
			result	= dbp_prehook_serverinfo(request);
			break;
	}
	return(result);
}