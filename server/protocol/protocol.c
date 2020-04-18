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

#ifdef PROFILE

ulong request_counter	= 0;

#endif

void dbp_connection_accept_loop(dbp_protocol_s *protocol)
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

		for(int error = 0; error == SUCCESS;)
		{
#ifdef PROFILE
			request_counter++;

			if (request_counter > 1000000)
			{
				exit(1111);
			}
#endif
			dbp_response_s response	= {0};
			dbp_request_s request	= {0};
			protocol->current_response	= &response;
			protocol->current_request	= &request;
			response.instance	= (char*)protocol;
			request.instance	= (char*)protocol;

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
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_SERVER_SHUTDOWN);
}

void dbp_handle_close(dbp_request_s *request, dbp_response_s *response)
{
	config_free_all(attribs_parse, DBP_ATTRIBS_STRUCT_COUNT
		, (char*)&request->attribs);
	my_list_free(request->header_list);
	hash_table_free(request->header_table);
	m_free(request->header_raw.data_address, MEMORY_FILE_LINE);
	if (request->temp_file.filename.address != NULL)
	{
		m_free(request->temp_file.filename.address, MEMORY_FILE_LINE);
	}
	my_list_free(response->header_list);
}

int dbp_handle_response(dbp_response_s *response, enum dbp_response_code code)
{
	switch (code)
	{
	case DBP_RESPONSE_SUCCESS: // alias to SUCCESS
	{
		return(SUCCESS);
	}
	case DBP_RESPONSE_DATA_SEND:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_DATA_SEND);
	}
	break;
	
	case DBP_RESPONSE_PACKET_OK:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_PACKET_OK);
	}
	break;

	case DBP_RESPONSE_ACTION_INVALID:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_ACTION_INVALID);
	}
	break;

	case DBP_RESPONSE_HEADER_EMPTY:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_HEADER_EMPTY);
	}
	break;

	case DBP_RESPONSE_PARSE_ERROR:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_PARSE_ERROR);
	}
	break;

	case DBP_RESPONSE_THIN_ATTRIBS:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_THIN_ATTRIBS);
	}
	break;

	case DBP_RESPONSE_ATTRIB_VALUE_INVALID:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_ATTIB_VALUE_INVALID);
	}
	break;

	case DBP_RESPONSE_FILE_EXISTS_ALREADY:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_FILE_EXISTS_ALREADY);
	}
	break;

	case DBP_RESPONSE_FILE_NOT_FOUND:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_FILE_NOT_FOUND);
	}
	break;

	case DBP_RESPONSE_FILE_UPDATE_OUTOFBOUNDS:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_FILE_UPDATE_OUTOFBOUNDS);
	}
	break;

	case DBP_RESPONSE_CORRUPTED_PACKET:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_CORRUPTED_PACKET);
	}
	break;

	case DBP_RESPONSE_CORRUPTED_DATA_HEADERS:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_CORRUPTED_DATA_HEADERS);
	}
	break;

	case DBP_RESPONSE_SETUP_ENV_FAILED:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_SETUP_ENV_FAILED);
	}
	break;

	case DBP_RESPONSE_GENERAL_SERVER_ERROR:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_GENERAL_SERVER_ERROR);
	}
	break;
	}

	response->response_code	= code;

	if (dbp_response_write(response) != SUCCESS)
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
	if(shutdown(protocol.connection.client, SHUT_RDWR) == 0)
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

void dbp_close(dbp_protocol_s protocol)
{
	close(protocol.connection.client);
	close(protocol.connection.server);
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DBP_CONNECTION_SHUTDOWN_CLEANUP);
	logs_close();
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
	request->header_table   = dbp_header_hash(request->header_list);

	result	= dbp_request_read_action(request);
	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}

	enum dbp_attribs_enum *asserts = 
		dbp_call_asserts[request->action - DBP_ATTRIB_ACTION];
	boolean assert	= dbp_attribs_assert(request->header_table, asserts
		, DBP_ACTIONS_COUNT);

	if (assert == FALSE)
	{
		return(DBP_RESPONSE_THIN_ATTRIBS);
	}
	
	//right after we have confirmed that all the required attribs
	//are present for the function call to succeed, we can parse them
	//to the structure for dbp_common_attribs_s
	
	dbp_protocol_attribs_s dbp_parsed_attribs	= {0};

	config_read_all(request->header_list
		, attribs_parse
		, DBP_ATTRIBS_STRUCT_COUNT
		, (char*)&dbp_parsed_attribs);
	
	request->attribs	= dbp_parsed_attribs;

	result = dbp_action_prehook(request);

	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}

	if (dbp_handle_response(response, DBP_RESPONSE_DATA_SEND) != SUCCESS)
	{
		return(DBP_RESPONSE_ERROR_WRITE);
	}

	if (request->header_info.data_length) 
	{
		if (dbp_file_setup_environment() == SUCCESS) 
		{
			result	= dbp_request_data(protocol, request);
			if (result != DBP_RESPONSE_SUCCESS)
			{
				return(result);
			}
		} 
		else 
		{
			printf("setting up env failed. ");
			return(DBP_RESPONSE_SETUP_ENV_FAILED);
		}
	}

	result	= dbp_action_posthook(request, response);
	
	if (result != DBP_RESPONSE_SUCCESS)
	{
		return(result);
	}
	
	// result	= dbp_response_write(response);
	
	// if (result != DBP_CONNECTION_NOERROR)
	// {
	// 	return(result);
	// }

	if (dbp_handle_response(response, DBP_RESPONSE_PACKET_OK) != SUCCESS)
	{
		return(DBP_RESPONSE_ERROR_WRITE);
	}

	m_free(request->temp_file.filename.address, MEMORY_FILE_LINE);
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
	}
	return(result);
}