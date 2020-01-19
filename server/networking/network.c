// networking.c

// This file will contain all the routines required to interact with the client. 
// No client communication can be made without this file, but in the same way
// this file should not handle anything other than processing client communication.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "network.h"
#include "../memory.h"
// #include <errno.h>

int assert_connection(netconn_info_s *conn
                    , int compare1
                    , int compare2
                    , char *function_string
                    , int error_num)
{
    if (compare1 == compare2) {
		printf("%s () failed with error: %d, errno:%d\n"
                , function_string, compare1, error_num);
		perror(function_string);

        if (conn->connection_status > NETWORK_STATUS_NOINIT)
            close(conn->server);

        conn->connection_status = NETWORK_STATUS_ERROR;
        conn->error_code    = error_num;
        return(0);
	}
    return(1);
}

// The function will make the program wait for the client to connect. 
// And it will not return until a client is connected. 
netconn_info_s network_connect_init_sync(int port)
{
    netconn_info_s connection = {0};

    connection.server =  socket(AF_INET, SOCK_STREAM, 0);
    if(!assert_connection(&connection, connection.server, INVALID_SOCKET
                    , "socket", SERVER_SOCK_INIT_FAILED))
        return connection;

    connection.connection_status = NETWORK_STATUS_SOCKETAVAIL;

    connection.server_socket.sin_addr.s_addr    = INADDR_ANY;
    connection.server_socket.sin_family         = AF_INET;
    connection.server_socket.sin_port           = htons(port);

    int result = 0;
    result = bind(connection.server
                        , (struct sockaddr*) &connection.server_socket
                        , sizeof(struct  sockaddr_in));
    if(!assert_connection(&connection, result, BIND_ERROR
                    , "bind", SERVER_BIND_FAILED))
        return connection;
        
    result = listen(connection.server, MAX_LISTEN_QUEQUE);
    if(!assert_connection(&connection, result, GENERAL_ERROR
                    , "listen", SERVER_LISTEN_FAILED))
        return connection;

    connection.connection_status = NETWORK_STATUS_LISTENING;
    return connection;
}

int network_connect_accept_sync(netconn_info_s *connection)
{
    socklen_t struct_len = sizeof(struct sockaddr_in);

	connection->client	= accept(connection->server
                            , (struct sockaddr*) &connection->client_socket
							, &struct_len);
    
    if(!assert_connection(connection, connection->client, GENERAL_ERROR
                            , "accept", SERVER_ACCEPT_FAILED))
        return(FAILED);
    
    // connection falls immediately and automatically from 
    // NETWORK_STATUS_CONNECTED to NETWORK_STATUS_READYWAIt after "accept"
    connection->connection_status   = NETWORK_STATUS_READYWAIT;

    return(SUCCESS);
}

netconn_data_s network_data_readxbytes(netconn_info_s *conn, int size)
{
    netconn_data_s  data = {0};

    // First thing we want to make sure is that the connection is still open.  
    if(conn->connection_status != NETWORK_STATUS_READYWAIT) {
        data.read_status   = DATA_READ_ERROR;
        data.error_code    = SERVER_NETWORK_STATUS_NONVALID;
        return data;
    }

    // This is how spare is intended to be used, as regular memory buffer.
    void *memory = 0;
    if(size >= 0 && size <= 8) {
        memory  = (void*) &data.spare;
        data.read_status = DATA_READ_SPARE;
    }
    else if (size < MAX_ALLOWED_NETWORK_BUFFER) {
        memory  = m_malloc(size, "network.c:net**readxbytes");
        data.read_status = DATA_READ_MALLOC;
    }
    else 
        exit(SERVER_OUT_OF_MEMORY);

    data.data_address = memory;
    
    int data_read = 0;
    while (data_read < size) {
        int length  = read(conn->client, memory + data_read, (size - data_read));

        if (length == 0) {
            data.read_status |= DATA_READ_EOF;
            break;
        }
        else if (length < 0) {
            data.read_status |= DATA_READ_ERROR;
            break;
        }
        else data_read   += length;
    }
    
    return (data);
}

int network_connection_write(netconn_info_s *conn, char *data, int length)
{
    int data_written = 0;
    while(data_written < length) {
        int written = write(conn->client, data + data_written
                                , length - data_written);
        
        if(written < 1) {
            return(FAILED);
        } else data_written += written;
    }
    return(SUCCESS);
}

#define NETWORK_DATA_READ_TYPE(data_type) \
netconn_data_basetypes_s network_data_read_##data_type(netconn_info_s *conn) \
{ \
    netconn_data_basetypes_s base_data = {0}; \
    netconn_data_s data = (network_data_readxbytes(conn, sizeof(data_type))); \
    base_data.data_source = data; \
    if (data.read_status == DATA_READ_SPARE) { \
        base_data.data_u._##data_type = *(data_type*)&data.spare; \
        return base_data; \
    } \
    base_data.read_status   = DATA_READ_NO_TYPE(data.read_status); \
    return base_data; \
}

NETWORK_DATA_READ_TYPE(char);
NETWORK_DATA_READ_TYPE(short);
NETWORK_DATA_READ_TYPE(int);
NETWORK_DATA_READ_TYPE(long);

/* example preprocessed:
netconn_data_basetypes_s network_data_read_long(netconn_info_s *conn) 
{
    netconn_data_basetypes_s base_data = {0}; 
    netconn_data_s data = (network_data_readxbytes(conn, sizeof(long)));
    base_data.data_source = data; 
    if (data.read_status == DATA_READ_SPARE) 
    {
         base_data.data_u._long = *(long*)&data.spare;
         return base_data; 
    }
    base_data.read_status = DATA_READ_NO_TYPE(data.read_status);
    return base_data; 
}
*/