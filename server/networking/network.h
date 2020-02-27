# ifndef NETWORK_INCLUDE_GAURD
# define NETWORK_INCLUDE_GAURD

#define __NETWORK__

#include <sys/socket.h>
#include <arpa/inet.h>

#define APPLICATION_PORT    4096
#define MAX_LISTEN_QUEQUE   8

/**
 * server - server file pointer
 * client - client file pointer
 * server_socket -  socket struct sockaddr_in for the server
 * client_socket -  socket struct sockaddr_in for the client
 * error_code -     int to indicate the error code 
 * is_setup_complete -  boolean variable to indicate that the 
 *                      structure is available to use.
 */
typedef struct network_connection_information
{
    int server;
    int client;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;
    char is_setup_complete;
    int error_code;
} netconn_info_s;

 /*
  * If the amount of data read is less than 8 bytes the data will be
  * read into the "spare" variable, if memory needs to be allocated
  * the caller should free the memory.
  */
typedef struct network_connection_data_read
{
    char is_spare;
    char is_malloc;
    char is_file;
    char is_error;
    char *data_address;
    size_t data_length;
    int error_code;
    char spare[8];
} netconn_data_s;

/**
 * data_types_s: for network read less than 8 bytes, where
 * value can be converted to a C type such as char, short, int or long. 
 * we will be able to do the converstion and use this struct.
 */
typedef struct {
    union
    {
        char _char;
        short _short;
        int _int;
        long int _long;
    } data_types_u;
    char is_error;
} data_types_s;

netconn_info_s network_connect_init_sync(int);
int network_connect_accept_sync(netconn_info_s*);
void network_data_free(netconn_data_s conn);

netconn_data_s network_data_readstream(netconn_info_s *conn, int size);
int network_connection_write(netconn_info_s *conn, char *data, int length);
char *network_data_address(netconn_data_s *data);

#define NETWORK_DATA_READ_TYPE_DECLARATION(data_type) \
data_types_s network_data_read_##data_type(netconn_info_s *conn);

NETWORK_DATA_READ_TYPE_DECLARATION(char);
NETWORK_DATA_READ_TYPE_DECLARATION(short);
NETWORK_DATA_READ_TYPE_DECLARATION(int);
NETWORK_DATA_READ_TYPE_DECLARATION(long);

# endif