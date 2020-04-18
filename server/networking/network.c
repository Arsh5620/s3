// networking.c

/*
 * This file will contain all the routines required to interact with the client. 
 * no client communication should be made outside of this file, but in the same way
 * this file should not handle anything other than processing client communication.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>

#include "network.h"
#include "../general/defines.h"
#include "../memdbg/memory.h"
#include "../errors/errorhandler.h"

/*
 * return value: 0 for success, any other value for error
 * side-effects: this function will call exit() if compare1 != compare2
 * it will then write the error for the last called function to both
 * standard error and log output.
 */
int assert_connection(int compare1, int compare2
	, char *function_string, int error_num)
{
	if (compare1 == compare2) {
		char *error  = strerror(errno);
		error_handle(ERRORS_HANDLE_STDOLOG
			, LOG_LEVEL_EXIT_SET(LOGGER_LEVEL_CATASTROPHIC
			, SERVER_ERROR_REFER_TO_LOGS)
			, NETWORK_ASSERT_MESSAGE, function_string
			, error, errno);
	}
	return(SUCCESS);
}

/*
 * this function will initialize a new socket, bind it, and then 
 * setup listening on the port, it will not accept a connection,
 * to accept connections call network_connect_accept_sync()
 */
network_s network_connect_init_sync(int port)
{
	network_s connection = {0};

	connection.server =  socket(AF_INET, SOCK_STREAM, 0);
	assert_connection(connection.server, INVALID_SOCKET
					, "socket", SERVER_SOCK_INIT_FAILED);

	connection.server_socket.sin_addr.s_addr    = INADDR_ANY;
	connection.server_socket.sin_family         = AF_INET;
	connection.server_socket.sin_port           = htons(port);

#ifndef RELEASE_BUILD
	int enable	= 1;
	if (setsockopt(connection.server, SOL_SOCKET
		, SO_REUSEADDR, &enable, sizeof(int)))
	{
		printf("Setting sock address reuse failed.");
		exit(EXIT_FAILURE);
	}

	int r1 = setsockopt(connection.server, IPPROTO_TCP
		, TCP_NODELAY, (char *) &enable, sizeof(int)); 
	if (r1 < 0 )
	{
		printf("Setting sock address reuse failed.");
		exit(EXIT_FAILURE);
	}
#endif

	int result = 0;
	result = bind(connection.server
		, (struct sockaddr*) &connection.server_socket
		, sizeof(struct  sockaddr_in));
	assert_connection(result, BIND_ERROR, "bind", SERVER_BIND_FAILED);

	result = listen(connection.server, NETWORK_LISTEN_QUEQUE);
	assert_connection(result, GENERAL_ERROR, "listen", SERVER_LISTEN_FAILED);

	error_handle(ERRORS_HANDLE_LOGS, LOGGER_LEVEL_INFO
		, NETWORK_PORT_LISTENING
		, port , NETWORK_LISTEN_QUEQUE);

	return connection;
}

/*
 * return values: 0 for success, any other value for error
 * this function accepts the netconn_info_s structure returned
 * by network_connect_init_sync. 
 * this function is sync and will block until a client connects.
 */
int network_connect_accept_sync(network_s *connection)
{
	socklen_t len = sizeof(struct sockaddr_in);

	connection->client	= accept(connection->server
		, (struct sockaddr*) &connection->client_socket, &len);
	
	// if assert fails as a side effect it will call exit()
	assert_connection(connection->client, GENERAL_ERROR
							, "accept", SERVER_ACCEPT_FAILED);
	
	return(SUCCESS);
}

/*
 * return type: netconn_data_s which will have all data information
 * if required this function will automatically perform dynamic
 * memory allocation, it is best to call network_data_free after 
 * the function to release any dynamic memory once it is no longer 
 * in use. 
 */
network_data_s network_read_stream(network_s *connection, ulong size)
{
	network_data_s  data = {0};

	char *memory = 0;
	if (size > NETWORK_READ_BUFFER)
	{
		size	= NETWORK_READ_BUFFER;
		data.error_code = SERVER_OUT_OF_MEMORY;
	}
	memory  = m_malloc(size, MEMORY_FILE_LINE);

	size_t data_read = 0;
	for (; data_read < size;)
	{
		ulong length  = read(connection->client
			, memory + data_read, (size - data_read));

		if (length == 0) 
		{
			if (data_read != size)
			{
				data.error_code	= NETWORK_ERROR_READ_PARTIAL;
			} 
			else 
			{
				data.error_code	= NETWORK_ERROR_SUCCESS;
			}
			break;
		}
		else if (length < 0) 
		{
			switch (errno)
			{
			case ECONNRESET:
				data.error_code	= NETWORK_ERROR_READ_CONNRESET;
				break;
			
			default:
				data.error_code	= NETWORK_ERROR_READ_ERROR;
				break;
			}
			break;
		}
		else
		{
			data_read	+= length;
		}
	}

	data.data_address	= memory;
	data.data_length	= data_read;
	return (data);
}

int network_write_stream(network_s *network, char *buffer, ulong buffer_length)
{
	ulong bytes_written	= 0;
	for (ulong length = 0;bytes_written < buffer_length;)
	{
		length = write(network->client, buffer, buffer_length);
		if (length > 0)
		{
			bytes_written += length;
		}
	}
	
	if (bytes_written != buffer_length)
	{
		switch (errno)
		{
		case ECONNRESET:
			{
				return(NETWORK_ERROR_READ_CONNRESET);
			}
		break;

		default:
			{
				return(NETWORK_ERROR_READ_ERROR);
			}
		break;
		}
	}
	return(NETWORK_ERROR_SUCCESS);
}

/*
 * releases memory (if-any) allocated by network_data_readxbytes
 */
void network_data_free(network_data_s data)
{
	if (data.data_address)
	{
		m_free(data.data_address, MEMORY_FILE_LINE); 
	}
}

#define NETWORK_READ(type) \
network_data_atom_s network_read_##type(network_s *network) \
{ \
	network_data_s	d0	= network_read_stream(network, sizeof(type));\
	network_data_atom_s d1	= {0}; \
	if (d0.error_code == NETWORK_ERROR_SUCCESS) \
	{ \
		d1._u.type##_t = *(type*) d0.data_address; \
	} \
	d1.error_code	= d0.error_code; \
	network_data_free(d0); \
	return d1; \
}

// basically instead of copy pasting the same code 4 times
// we have compiler copy paste the code, so we can change once and done.

NETWORK_READ(char);
NETWORK_READ(short);
NETWORK_READ(int);
NETWORK_READ(long);

/* example:
network_data_atom_s network_read_int(network_s *network) 
{ 
	network_data_s d0 = network_read_stream(network, sizeof(int));
	network_data_atom_s d1 = {0};
	if (d0.error_code) 
	{ 
		d1._u.int_t = *(int*) d0.data_address; 
	}
	d1.error_code = d0.error_code;
	return d1; 
}
*/