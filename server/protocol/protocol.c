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

		for(enum dbp_errors_enum error = 0;
			error == DBP_CONNECTION_NOERROR || error == DBP_CONNECTION_WARN;)
		{
			dbp_response_s response	= {0};
			dbp_request_s request	= {0};
			protocol->current_response	= &response;
			protocol->current_request	= &request;
			response.instance	= (char*)protocol;
			request.instance	= (char*)protocol;

			error	= dbp_protocol_nextrequest(protocol);

			printf("dbp_next returned value: %d\n", error);

			error	= dbp_handle_warns(protocol, error);
			dbp_handle_errors(&response, error, &shutdown);
		}

		dbp_shutdown_connection(*protocol, shutdown);
	}
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, PROTOCOL_SERVER_SHUTDOWN);
}

void dbp_handle_errors(dbp_response_s *response, 
	enum dbp_errors_enum error, int *shutdown)
{
	if (error != DBP_CONNECTION_NOERROR && error!= DBP_CONNECTION_WARN)
	{
		switch (error)
		{
		case DBP_CONNECTION_ERROR_CORRUPTION:
		{
			response->response_code	= DBP_RESPONSE_CORRUPTED_PACKET;
			response->data_string	=
				STRING_S(DBP_RESPONSE_STRING_HEADER_CORRUPTED);
		}
		break;

		case DBP_CONNECTION_ERROR_DATAHEADERS:
		{
			response->response_code	= 
				DBP_RESPONSE_CORRUPTED_DATAHEADERS;
			response->data_string	=
				STRING_S(DBP_RESPONSE_STRING_DATA_HEADER_CORRUPTED);
		}
		break;
		
		case DBP_CONNECTION_ERROR_ENV_FAILED:
		{
			response->response_code	= DBP_RESPONSE_SETTING_UP_ENV_FAILED;
			response->data_string	=
				STRING_S(DBP_RESPONSE_STRING_ENV_FAILED);
		}
		break;
		default:
			break;
		}

		dbp_response_write(response);
		*shutdown	= DBP_CONNECTION_SHUTDOWN_CORRUPTION;
	}
}

int dbp_handle_warns(dbp_protocol_s *protocol, enum dbp_warns_enum warn)
{
	dbp_response_s *response 	= protocol->current_response;
	/* Here we will try to recover the connection after a possible crash */
	switch (warn)
	{
	/**
	 * read all the data from the socket, and tell the 
	 * client that action is not supported 
	 * */
	case DBP_CONNECTION_WARN_ACTION_INVALID:
	{
		response->response_code	= DBP_RESPONSE_NOT_AN_ACTION;
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_INVALID_ACTION);
	}
	break;

	/**
	 * connection header is empty and does not contain key value pairs. 
	 * reply to the client with connection empty error
	 */
	case DBP_CONNECTION_WARN_EMPTY:
	{
		response->response_code	= DBP_RESPONSE_EMPTY_PACKET;
		response->data_string	= STRING_S(DBP_RESPONSE_STRING_EMPTY);
	}
	break;

	/**
	 * There was an error parsing the header, read all the data and clear
	 * the connection for further use. 
	 */
	case DBP_CONNECTION_WARN_PARSEERROR:
	{
		response->response_code	= DBP_RESPONSE_PARSER_ERROR;
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_PARSER_ERROR);
	}
	break;

	/**
	 * header is missing some of the required attributes for the action
	 * send the client response with all "required" attributes. 
	 */
	case DBP_CONNECTION_WARN_THIN_ATTRIBS:
	{
		response->response_code	= DBP_RESPONSE_NOT_ENOUGH_ATTRIBS;
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_NOT_ENOUGH_ATTRIBS);
	}	
	break;
	
	/**
	 * headers are correct, we can start accepting data now. 
	 * Until now, the client should not have started sending data
	 */
	case DBP_CONNECTION_NOWARN:
	{
	 	// IGNORE.
		return(DBP_CONNECTION_WARN);
	}
	break;
	default:
		// other warnings must be treated as error, and program will exit
		return(warn);
	}

	dbp_response_write(response);
	return(DBP_CONNECTION_WARN);
}

int dbp_handle_response(dbp_response_s *response, enum dbp_response_code code)
{
	switch (code)
	{
	case DBP_RESPONSE_DATA_SEND:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_SEND_DATA);
	}
	break;
	
	case DBP_RESPONSE_PACKET_OK:
	{
		response->data_string	= 
			STRING_S(DBP_RESPONSE_STRING_PACKET_OK);
	}
	break;
	default:
		break;
	}

	response->response_code	= code;
	return(dbp_response_write(response));
}

void dbp_shutdown_connection(dbp_protocol_s protocol
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

void dbp_cleanup(dbp_protocol_s protocol)
{
	close(protocol.connection.client);
	close(protocol.connection.server);
	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
			, DBP_CONNECTION_SHUTDOWN_CLEANUP);
	logs_close();
}

// returns 0 for no-error, any other number for error or conn close request
ulong dbp_protocol_nextrequest(dbp_protocol_s *protocol)
{
	dbp_request_s *request	= protocol->current_request;
	dbp_response_s *response	= protocol->current_response;

	int result	= dbp_request_readheaders(*protocol, request);
	if (result != DBP_CONNECTION_NOERROR)
	{
		return(result);
	}
	
	request->instance	= (char*)protocol;
	request->header_table   = dbp_headers_make_table(request->header_list);

	result	= dbp_read_action(request);
	if (result != DBP_CONNECTION_NOERROR)
	{
		return(result);
	}

	enum dbp_attribs_enum *asserts = 
		dbp_call_asserts[request->action - DBP_ATTRIB_ACTION];
	boolean assert	= dbp_list_assert(request->header_table, asserts
		, DBP_ACTIONS_COUNT);

	if (assert == FALSE)
	{
		return(DBP_CONNECTION_WARN_THIN_ATTRIBS);
	}
	
	//right after we have confirmed that all the required attribs
	//are present for the function call to succeed, we can parse them
	//to the structure for dbp_common_attribs_s
	
	dbp_protocol_attribs_s dbp_parsed_attribs	= {0};

	config_read_all(request->header_list
		, attribs_parse
		, DBP_ATTRIBS_COUNT
		, (char*)&dbp_parsed_attribs);

	result = dbp_action_prehook(request);

	if (result != DBP_CONNECTION_NOERROR)
	{
		return(result);
	}

	if (dbp_handle_response(response, DBP_RESPONSE_DATA_SEND) != SUCCESS)
	{
		return(DBP_CONNECTION_ERROR_WRITEFAILED);
	}

	if (request->header_info.data_length) 
	{
		if (dbp_setup_environment() == SUCCESS) 
		{
			result	= dbp_request_data(protocol, request);
			if (result != DBP_CONNECTION_NOERROR)
			{
				return(result);
			}
		} 
		else 
		{
			printf("setting up env failed. ");
			return DBP_CONNECTION_ERROR_ENV_FAILED;
		}
	}

	result	= dbp_action_posthook(request, response);
	
	if (result != DBP_CONNECTION_NOERROR)
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
		return(DBP_CONNECTION_ERROR_WRITEFAILED);
	}

	m_free(request->temp_file.filename.address, MEMORY_FILE_LINE);
	return(DBP_CONNECTION_NOERROR);
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