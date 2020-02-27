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

#include "network.h"
#include "../defines.h"
#include "../memory.h"
#include "../logs.h"

/*
 * return value: 0 for success, any other value for error
 * side-effects: this function will call exit() if compare1 != compare2
 * it will then write the error for the last called function to both
 * standard error and log output.
 */
int assert_connection(netconn_info_s *conn
                    , int compare1
                    , int compare2
                    , char *function_string
                    , int error_num)
{
    if (compare1 == compare2) {
		char *error_string  = strerror(errno);

		fprintf(stderr, "%s () failed with error: %s, errno:%d\n"
                , function_string, error_string, error_num);

        logs_write_printf("%s did not succeed, error"
            " message: %s, errno: %d, error-code: %d\n"
            , function_string, error_string, errno, error_num);
            
        exit(SERVER_ERROR_REFER_TO_LOGS);
	}
    return(SUCCESS);
}

/*
 * this function will initialize a new socket, bind it, and then 
 * setup listening on the port, it will not accept a connection,
 * to accept connections call network_connect_accept_sync()
 */
netconn_info_s network_connect_init_sync(int port)
{
    netconn_info_s connection = {0};

    connection.server =  socket(AF_INET, SOCK_STREAM, 0);
    if(assert_connection(&connection, connection.server, INVALID_SOCKET
                    , "socket", SERVER_SOCK_INIT_FAILED))
        return connection;

    connection.server_socket.sin_addr.s_addr    = INADDR_ANY;
    connection.server_socket.sin_family         = AF_INET;
    connection.server_socket.sin_port           = htons(port);

    int result = 0;
    result = bind(connection.server
                        , (struct sockaddr*) &connection.server_socket
                        , sizeof(struct  sockaddr_in));
    if(assert_connection(&connection, result, BIND_ERROR
                    , "bind", SERVER_BIND_FAILED))
        return connection;
        
    result = listen(connection.server, MAX_LISTEN_QUEQUE);
    if(assert_connection(&connection, result, GENERAL_ERROR
                    , "listen", SERVER_LISTEN_FAILED))
        return connection;

    connection.is_setup_complete    = TRUE;
    return connection;
}

/*
 * return values: 0 for success, any other value for error
 * this function accepts the netconn_info_s structure returned
 * by network_connect_init_sync. 
 * this function is sync and will block until a client connects.
 */
int network_connect_accept_sync(netconn_info_s *connection)
{
    socklen_t struct_len = sizeof(struct sockaddr_in);

	connection->client	= accept(connection->server
                            , (struct sockaddr*) &connection->client_socket
							, &struct_len);
    
    // if assert fails as a side effect it will call exit()
    assert_connection(connection, connection->client, GENERAL_ERROR
                            , "accept", SERVER_ACCEPT_FAILED);
    
    return(SUCCESS);
}

/*
 * return type: netconn_data_s which will have all data information
 * if required this function will automatically perform dynamic
 * memory allocation, it is best to call network_data_free after 
 * the function to release any dynamic memory once it is no longer 
 * in use. 
 * ***
 * any read operations for less than 8 bytes does not require malloc. 
 * the is_spare variable is set to TRUE and you can read data from 
 * (char*)netconn_data_s.spare[8];
 */
netconn_data_s network_data_readstream(netconn_info_s *conn, int size)
{
    netconn_data_s  data = {0};

    char *memory = 0;

    if(size <=8 && size >=0) {
        memory  = (char*) data.spare;
        data.is_spare   = TRUE;
    }
    else if (size < MAX_ALLOWED_NETWORK_BUFFER) {
        memory  = m_malloc(size, MEMORY_FILE_LINE);
        data.is_malloc  = TRUE;
    }
    else {
        data.is_error   = TRUE;
        data.error_code = SERVER_OUT_OF_MEMORY;
        return(data);
    }

    size_t i = 0;
    for (; i < size;)
    {
        int length  = read(conn->client, memory + i, (size - i));

        if (length == 0) {
            if((i + length) != size){
                data.is_error   = TRUE;
            }
            break;
        }
        else if (length < 0) {
            data.is_error   = TRUE;
            break;
        }
        else i  += length;
    }

    data.data_address   = memory;
    data.data_length    = i;
    return (data);
}

/*
 * return type: char*, address for the return data buffer.
 * helper method to find if netconn_data_s.spare or .data_address
 * should be used by checking the is_spare and is_malloc variables.
 */
char *network_data_address(netconn_data_s *data)
{
    if(data->is_spare == TRUE)
        return (char*)&data->spare;
    else if(data->is_malloc == TRUE)
        return (data->data_address);
    else 
        return (0);
}

/*
 * releases memory (if-any) allocated by network_data_readxbytes
 */
void network_data_free(netconn_data_s data)
{
    if(data.is_malloc)
        m_free(data.data_address, MEMORY_FILE_LINE); 
}

/* 
 * return value: 0 for success, any other value for error.
 * this function basically performs network write to the client connected. 
 * the write will be a application layer RAW write over TCP/IP.
 */
int network_connection_write(netconn_info_s *conn, char *data, int length)
{
    int data_written = 0;
    while (data_written < length) {
        int written = write(conn->client, data + data_written
                                , length - data_written);
        
        if (written < 1)
            return(FAILED);
        else 
            data_written += written;
    }
    return(SUCCESS);
}

#define NETWORK_DATA_READ_TYPE(data_type) \
data_types_s network_data_read_##data_type(netconn_info_s *conn) \
{ \
    data_types_s type_s = {0}; \
    netconn_data_s data = (network_data_readstream(conn\
        , sizeof(data_type))); \
    if (data.is_spare == TRUE) { \
        type_s.data_types_u._##data_type = *(data_type*)&data.spare; \
    } \
    else { \
        type_s.is_error = TRUE; \
    } \
    return type_s; \
}

// basically instead of copy pasting the same code 4 times
// we have compiler copy paste the code, so we can change once and done.

NETWORK_DATA_READ_TYPE(char);
NETWORK_DATA_READ_TYPE(short);
NETWORK_DATA_READ_TYPE(int);
NETWORK_DATA_READ_TYPE(long);

/* example:
data_types_s network_data_read_char(netconn_info_s *conn) 
{ 
    data_types_s type_s = {0}; 
    netconn_data_s data = (network_data_readxbytes(conn, sizeof(char)));
    if (data.is_spare == TRUE) { 
        type_s.data_types_u._char = *(char*)&data.spare; 
    } else { 
        type_s.is_error = TRUE; 
    } 
    return type_s; 
}*/