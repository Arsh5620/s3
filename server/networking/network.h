# ifndef NETWORK_INCLUDE_GAURD
# define NETWORK_INCLUDE_GAURD

#define __NETWORK__

#include <sys/socket.h>
#include <arpa/inet.h>
#include "../general/defines.h"

#define NETWORK_PORT			4096
#define NETWORK_LISTEN_QUEQUE	8
#define NETWORK_READ_BUFFER		(8 MB) /* 8 * 1024 * 1024 */

enum network_errors_enum {
	NETWORK_ERROR_SUCCESS	= 0
	, NETWORK_ERROR_READ_PARTIAL = 128
	, NETWORK_ERROR_READ_CONNRESET
	, NETWORK_ERROR_READ_ERROR
};


/**
 * server - server file pointer
 * client - client file pointer
 * server_socket -  socket struct sockaddr_in for the server
 * client_socket -  socket struct sockaddr_in for the client
 * error_code -     int to indicate the error code 
 */
typedef struct network_connection
{
	int server;
	int client;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	uint error_code;
} network_s;


typedef struct network_connection_data
{
	char *data_address;
	ulong data_length;
	ulong data_maxlen;
	uint error_code;
} network_data_s;

/**
 * data_types_s: for network read less than 8 bytes, where
 * value can be converted to a C type such as char, short, int or long. 
 * we will be able to do the converstion and use this struct.
 */
typedef struct {
	union {
		char char_t;
		short short_t;
		int int_t;
		long long_t;
	} _u;
	uint error_code;
} network_data_atom_s;

network_s network_connect_init_sync(int port);
int network_connect_accept_sync(network_s *connection);
void network_data_free(network_data_s data);

network_data_s network_read_stream(network_s *connection, ulong size);
int network_write_stream(network_s *network, char *buffer, ulong buffer_length);

#define NETWORK_READ_DECLARE(type) \
network_data_atom_s network_read_##type(network_s *network);

NETWORK_READ_DECLARE(char);
NETWORK_READ_DECLARE(short);
NETWORK_READ_DECLARE(int);
NETWORK_READ_DECLARE(long);

# endif